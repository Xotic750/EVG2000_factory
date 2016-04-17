/* Masquerade.  Simple mapping which alters range to a local IP address
   (depending on route). */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <net/route.h>
#include <linux/netfilter_ipv4.h>
#ifdef CONFIG_NF_NAT_NEEDED
#include <net/netfilter/nf_nat_rule.h>
#else
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#endif
#include <linux/netfilter/x_tables.h>

#if defined(CONFIG_MIPS_BRCM)
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_nat_helper.h>
#endif /* CONFIG_MIPS_BRCM */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("iptables MASQUERADE target module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/* Lock protects masq region inside conntrack */
static DEFINE_RWLOCK(masq_lock);

#if defined(CONFIG_MIPS_BRCM)
/****************************************************************************/
static void bcm_nat_expect(struct nf_conn *ct,
			   struct nf_conntrack_expect *exp)
{
	struct nf_nat_range range;

	/* This must be a fresh one. */
	BUG_ON(ct->status & IPS_NAT_DONE_MASK);

	/* Change src to where new ct comes from */
	range.flags = IP_NAT_RANGE_MAP_IPS;
	range.min_ip = range.max_ip =
		ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
	nf_nat_setup_info(ct, &range, NF_IP_POST_ROUTING);
	 
	/* For DST manip, map port here to where it's expected. */
	range.flags = (IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED);
	range.min = range.max = exp->saved_proto;
	range.min_ip = range.max_ip = exp->saved_ip;
	nf_nat_setup_info(ct, &range, NF_IP_PRE_ROUTING);
}

/****************************************************************************/
static int bcm_nat_help(struct sk_buff **pskb, unsigned int protoff,
			struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	struct nf_conn_help *help = nfct_help(ct);
	struct nf_conntrack_expect *exp;
	
	if (dir != IP_CT_DIR_ORIGINAL || help->expecting)
		return NF_ACCEPT;

	DEBUGP("bcm_nat: packet[%d bytes] %u.%u.%u.%u:%hu->%u.%u.%u.%u:%hu, "
	       "reply: %u.%u.%u.%u:%hu->%u.%u.%u.%u:%hu\n",
	       (*pskb)->len,
	       NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip),
	       ntohs(ct->tuplehash[dir].tuple.src.u.udp.port),
	       NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip),
	       ntohs(ct->tuplehash[dir].tuple.dst.u.udp.port),
	       NIPQUAD(ct->tuplehash[!dir].tuple.src.u3.ip),
	       ntohs(ct->tuplehash[!dir].tuple.src.u.udp.port),
	       NIPQUAD(ct->tuplehash[!dir].tuple.dst.u3.ip),
	       ntohs(ct->tuplehash[!dir].tuple.dst.u.udp.port));
	
	/* Create expect */
	if ((exp = nf_conntrack_expect_alloc(ct)) == NULL)
		return NF_ACCEPT;

	nf_conntrack_expect_init(exp, AF_INET, NULL,
				 &ct->tuplehash[!dir].tuple.dst.u3,
				 IPPROTO_UDP, NULL,
				 &ct->tuplehash[!dir].tuple.dst.u.udp.port);
	exp->flags = NF_CT_EXPECT_PERMANENT;
	exp->saved_ip = ct->tuplehash[dir].tuple.src.u3.ip;
	exp->saved_proto.udp.port = ct->tuplehash[dir].tuple.src.u.udp.port;
	exp->dir = !dir;
	exp->expectfn = bcm_nat_expect;

	/* Setup expect */
	nf_conntrack_expect_related(exp);
	DEBUGP("bcm_nat: expect setup\n");

	return NF_ACCEPT;
}

/****************************************************************************/
static struct nf_conntrack_helper nf_conntrack_helper_bcm_nat __read_mostly = {
	.list = LIST_HEAD_INIT(nf_conntrack_helper_bcm_nat.list),
	.name = "BCM-NAT",
	.me = THIS_MODULE,
	.max_expected = 1000,
	.timeout = 240,
	.tuple.src.l3num = AF_INET,
	.tuple.dst.protonum = IPPROTO_UDP,
	.mask.src.l3num = 0xFFFF,
	.mask.dst.protonum = 0xFF,
	.help = bcm_nat_help,
};

/****************************************************************************/
static inline int find_exp(u_int32_t ip, u_int16_t port, struct nf_conn *ct)
{
	struct nf_conntrack_expect * exp;
	
	list_for_each_entry(exp, &nf_conntrack_expect_list, list) {
		if (exp->tuple.dst.u3.ip == ip &&
		    exp->tuple.dst.u.all == port &&
		    exp->tuple.dst.protonum ==
		    ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)
		    	return 1;
	}
	return 0;
}

/****************************************************************************/
static inline struct nf_conntrack_expect *find_fullcone_exp(struct nf_conn *ct)
{
	struct nf_conntrack_expect * exp;
	struct nf_conntrack_tuple * tp =
		&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;

	list_for_each_entry(exp, &nf_conntrack_expect_list, list) {
		if (exp->saved_ip == tp->src.u3.ip &&
		    exp->saved_proto.all == tp->src.u.all &&
		    exp->tuple.dst.protonum == tp->dst.protonum &&
		    exp->tuple.src.u3.ip == 0 &&
		    exp->tuple.src.u.udp.port == 0)
			return exp;
	}
	return NULL;
}
#endif /* CONFIG_MIPS_BRCM */

/* FIXME: Multiple targets. --RR */
static int
masquerade_check(const char *tablename,
		 const void *e,
		 const struct xt_target *target,
		 void *targinfo,
		 unsigned int hook_mask)
{
	const struct ip_nat_multi_range_compat *mr = targinfo;

	if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS) {
		DEBUGP("masquerade_check: bad MAP_IPS.\n");
		return 0;
	}
	if (mr->rangesize != 1) {
		DEBUGP("masquerade_check: bad rangesize %u.\n", mr->rangesize);
		return 0;
	}
	return 1;
}

static unsigned int
masquerade_target(struct sk_buff **pskb,
		  const struct net_device *in,
		  const struct net_device *out,
		  unsigned int hooknum,
		  const struct xt_target *target,
		  const void *targinfo)
{
#ifdef CONFIG_NF_NAT_NEEDED
	struct nf_conn_nat *nat;
#endif
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;
	struct ip_nat_range newrange;
	const struct ip_nat_multi_range_compat *mr;
	struct rtable *rt;
	__be32 newsrc;

	IP_NF_ASSERT(hooknum == NF_IP_POST_ROUTING);

	ct = ip_conntrack_get(*pskb, &ctinfo);
#ifdef CONFIG_NF_NAT_NEEDED
	nat = nfct_nat(ct);
#endif
	IP_NF_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED
			    || ctinfo == IP_CT_RELATED + IP_CT_IS_REPLY));

	/* Source address is 0.0.0.0 - locally generated packet that is
	 * probably not supposed to be masqueraded.
	 */
#ifdef CONFIG_NF_NAT_NEEDED
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == 0)
#else
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip == 0)
#endif
		return NF_ACCEPT;

	mr = targinfo;
	rt = (struct rtable *)(*pskb)->dst;
	newsrc = inet_select_addr(out, rt->rt_gateway, RT_SCOPE_UNIVERSE);
	if (!newsrc) {
		printk("MASQUERADE: %s ate my IP address\n", out->name);
		return NF_DROP;
	}

	write_lock_bh(&masq_lock);
#ifdef CONFIG_NF_NAT_NEEDED
	nat->masq_index = out->ifindex;
#else
	ct->nat.masq_index = out->ifindex;
#endif
	write_unlock_bh(&masq_lock);
	
#if defined(CONFIG_MIPS_BRCM)
	if (mr->range[0].min_ip != 0 /* nat_mode == full cone */
	    && nfct_help(ct)->helper == NULL
	    && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum ==
	    IPPROTO_UDP) {
		unsigned int ret;
		u_int16_t minport;
		u_int16_t maxport;
		struct nf_conntrack_expect *exp;

		DEBUGP("bcm_nat: need full cone NAT\n");

		/* Choose port */
		read_lock_bh(&nf_conntrack_lock);
		exp = find_fullcone_exp(ct);
		if (exp) {
			minport = maxport = exp->tuple.dst.u.udp.port;
			DEBUGP("bcm_nat: existing mapped port = %hu\n",
			       ntohs(minport));
		} else { /* no previous expect */
			u_int16_t newport, tmpport;
			
			minport = mr->range[0].min.all == 0? 
				ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.
				u.udp.port : mr->range[0].min.all;
			maxport = mr->range[0].max.all == 0? 
				htons(65535) : mr->range[0].max.all;
			for (newport = ntohs(minport),tmpport = ntohs(maxport); 
			     newport <= tmpport; newport++) {
			     	if (!find_exp(newsrc, htons(newport), ct)) {
					DEBUGP("bcm_nat: new mapped port = "
					       "%hu\n", newport);
					minport = maxport = htons(newport);
					break;
				}
			}
		}
		read_unlock_bh(&nf_conntrack_lock);

		newrange = ((struct ip_nat_range)
			{ mr->range[0].flags | IP_NAT_RANGE_MAP_IPS |
			  IP_NAT_RANGE_MAP_IPS, newsrc, newsrc,
			  {.udp = {minport}}, {.udp = {maxport}}});
	
		/* Set ct helper */
		ret = nf_nat_setup_info(ct, &newrange, hooknum);
		if (ret == NF_ACCEPT) {
			nfct_help(ct)->helper = &nf_conntrack_helper_bcm_nat;
			DEBUGP("bcm_nat: helper set\n");
		}
		return ret;
	}
#endif /* CONFIG_MIPS_BRCM */

	/* Transfer from original range. */
	newrange = ((struct ip_nat_range)
		{ mr->range[0].flags | IP_NAT_RANGE_MAP_IPS,
		  newsrc, newsrc,
		  mr->range[0].min, mr->range[0].max });

	/* Hand modified range to generic setup. */
	return ip_nat_setup_info(ct, &newrange, hooknum);
}

static inline int
device_cmp(struct ip_conntrack *i, void *ifindex)
{
	int ret;
#ifdef CONFIG_NF_NAT_NEEDED
	struct nf_conn_nat *nat = nfct_nat(i);

	if (!nat)
		return 0;
#endif

	read_lock_bh(&masq_lock);
#ifdef CONFIG_NF_NAT_NEEDED
	ret = (nat->masq_index == (int)(long)ifindex);
#else
	ret = (i->nat.masq_index == (int)(long)ifindex);
#endif
	read_unlock_bh(&masq_lock);

	return ret;
}

static int masq_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	struct net_device *dev = ptr;

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		IP_NF_ASSERT(dev->ifindex != 0);

		ip_ct_iterate_cleanup(device_cmp, (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static int masq_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;

	if (event == NETDEV_DOWN) {
		/* IP address was deleted.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		IP_NF_ASSERT(dev->ifindex != 0);

		ip_ct_iterate_cleanup(device_cmp, (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static struct notifier_block masq_dev_notifier = {
	.notifier_call	= masq_device_event,
};

static struct notifier_block masq_inet_notifier = {
	.notifier_call	= masq_inet_event,
};

static struct xt_target masquerade = {
	.name		= "MASQUERADE",
	.family		= AF_INET,
	.target		= masquerade_target,
	.targetsize	= sizeof(struct ip_nat_multi_range_compat),
	.table		= "nat",
	.hooks		= 1 << NF_IP_POST_ROUTING,
	.checkentry	= masquerade_check,
	.me		= THIS_MODULE,
};

static int __init ipt_masquerade_init(void)
{
	int ret;

	ret = xt_register_target(&masquerade);

	if (ret == 0) {
		/* Register for device down reports */
		register_netdevice_notifier(&masq_dev_notifier);
		/* Register IP address change reports */
		register_inetaddr_notifier(&masq_inet_notifier);
	}

	return ret;
}

static void __exit ipt_masquerade_fini(void)
{
#if defined(CONFIG_MIPS_BRCM)
	nf_conntrack_helper_unregister(&nf_conntrack_helper_bcm_nat);
#endif
	xt_unregister_target(&masquerade);
	unregister_netdevice_notifier(&masq_dev_notifier);
	unregister_inetaddr_notifier(&masq_inet_notifier);
}

module_init(ipt_masquerade_init);
module_exit(ipt_masquerade_fini);
