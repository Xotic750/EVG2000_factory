/* SIP extension for UDP NAT alteration.
 *
 * (C) 2005 by Christian Hentschel <chentschel@arnet.com.ar>
 * based on RR's ip_nat_ftp.c and other modules.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <linux/netfilter/nf_conntrack_sip.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hentschel <chentschel@arnet.com.ar>");
MODULE_DESCRIPTION("SIP NAT helper");
MODULE_ALIAS("ip_nat_sip");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

struct addr_map {
	struct {
		char		src[sizeof("nnn.nnn.nnn.nnn:nnnnn")];
		char		dst[sizeof("nnn.nnn.nnn.nnn:nnnnn")];
		unsigned int	srclen, srciplen;
		unsigned int	dstlen, dstiplen;
	} addr[IP_CT_DIR_MAX];
};

static void addr_map_init(struct nf_conn *ct, struct addr_map *map)
{
	struct nf_conntrack_tuple *t;
	enum ip_conntrack_dir dir;
	unsigned int n;

	for (dir = 0; dir < IP_CT_DIR_MAX; dir++) {
		t = &ct->tuplehash[dir].tuple;

		n = sprintf(map->addr[dir].src, "%u.%u.%u.%u",
			    NIPQUAD(t->src.u3.ip));
		map->addr[dir].srciplen = n;
		n += sprintf(map->addr[dir].src + n, ":%u",
			     ntohs(t->src.u.udp.port));
		map->addr[dir].srclen = n;

		n = sprintf(map->addr[dir].dst, "%u.%u.%u.%u",
			    NIPQUAD(t->dst.u3.ip));
		map->addr[dir].dstiplen = n;
		n += sprintf(map->addr[dir].dst + n, ":%u",
			     ntohs(t->dst.u.udp.port));
		map->addr[dir].dstlen = n;
	}
}

static int map_sip_addr(struct sk_buff **pskb, enum ip_conntrack_info ctinfo,
			struct nf_conn *ct, const char **dptr, size_t dlen,
			enum sip_header_pos pos, struct addr_map *map)
{
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	unsigned int matchlen, matchoff, addrlen;
	char *addr;

	if (ct_sip_get_info(ct, *dptr, dlen, &matchoff, &matchlen, pos) <= 0)
		return 1;

	if ((matchlen == map->addr[dir].srciplen ||
	     matchlen == map->addr[dir].srclen) &&
	    memcmp(*dptr + matchoff, map->addr[dir].src, matchlen) == 0) {
		addr    = map->addr[!dir].dst;
		addrlen = map->addr[!dir].dstlen;
	} else if ((matchlen == map->addr[dir].dstiplen ||
		    matchlen == map->addr[dir].dstlen) &&
		   memcmp(*dptr + matchoff, map->addr[dir].dst, matchlen) == 0) {
		addr    = map->addr[!dir].src;
		addrlen = map->addr[!dir].srclen;
	} else
		return 1;

	if (!nf_nat_mangle_udp_packet(pskb, ct, ctinfo,
				      matchoff, matchlen, addr, addrlen))
		return 0;
	*dptr = (*pskb)->data + (*pskb)->nh.iph->ihl*4 + sizeof(struct udphdr);
	return 1;

}

#if defined(CONFIG_MIPS_BRCM)
static void nf_nat_sip_expect(struct nf_conn *new,
			      struct nf_conntrack_expect *exp)
{
	struct nf_nat_range range;

	/* This must be a fresh one. */
	BUG_ON(new->status & IPS_NAT_DONE_MASK);
	
	/* Change src to where new ct comes from */
	range.flags = IP_NAT_RANGE_MAP_IPS;
	range.min_ip = range.max_ip =
		new->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
	nf_nat_setup_info(new, &range, NF_IP_POST_ROUTING);

	/* For DST manip, map ip:port here to where it's expected. */
	range.flags = (IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED);
	range.min = range.max = exp->master->tuplehash[!exp->dir].tuple.src.u;
	range.min_ip = range.max_ip =
		exp->master->tuplehash[!exp->dir].tuple.src.u3.ip;
	nf_nat_setup_info(new, &range, NF_IP_PRE_ROUTING);
}
#endif /* CONFIG_MIPS_BRCM */

static unsigned int ip_nat_sip(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       struct nf_conn *ct,
#if defined(CONFIG_MIPS_BRCM)
			       struct nf_conntrack_expect *exp,
#endif /* CONFIG_MIPS_BRCM */
			       const char **dptr)
{
	int dir = CTINFO2DIR(ctinfo);
	enum sip_header_pos pos;
	struct addr_map map;
	int dataoff, datalen;

	dataoff = (*pskb)->nh.iph->ihl*4 + sizeof(struct udphdr);
	datalen = (*pskb)->len - dataoff;
	if (datalen < sizeof("SIP/2.0") - 1)
		return NF_ACCEPT;

#if defined(CONFIG_MIPS_BRCM)
	exp->expectfn = nf_nat_sip_expect;
	exp->dir = !dir;
	nf_conntrack_expect_related(exp);
#endif /* CONFIG_MIPS_BRCM */

	addr_map_init(ct, &map);

	/* Basic rules: requests and responses. */
	if (strncmp(*dptr, "SIP/2.0", sizeof("SIP/2.0") - 1) != 0) {
		/* 10.2: Constructing the REGISTER Request:
		 *
		 * The "userinfo" and "@" components of the SIP URI MUST NOT
		 * be present.
		 */
		if (datalen >= sizeof("REGISTER") - 1 &&
		    strncmp(*dptr, "REGISTER", sizeof("REGISTER") - 1) == 0)
			pos = POS_REG_REQ_URI;
		else
			pos = POS_REQ_URI;

		if (!map_sip_addr(pskb, ctinfo, ct, dptr, datalen, pos, &map))
			return NF_DROP;
	}

	if (!map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_FROM, &map) ||
	    !map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_TO, &map) ||
	    !map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_VIA, &map) ||
	    !map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_CONTACT, &map))
		return NF_DROP;
	return NF_ACCEPT;
}

static unsigned int mangle_sip_packet(struct sk_buff **pskb,
				      enum ip_conntrack_info ctinfo,
				      struct nf_conn *ct,
				      const char **dptr, size_t dlen,
				      char *buffer, int bufflen,
				      enum sip_header_pos pos)
{
	unsigned int matchlen, matchoff;

	if (ct_sip_get_info(ct, *dptr, dlen, &matchoff, &matchlen, pos) <= 0)
		return 0;

	if (!nf_nat_mangle_udp_packet(pskb, ct, ctinfo,
				      matchoff, matchlen, buffer, bufflen))
		return 0;

	/* We need to reload this. Thanks Patrick. */
	*dptr = (*pskb)->data + (*pskb)->nh.iph->ihl*4 + sizeof(struct udphdr);
	return 1;
}

static int mangle_content_len(struct sk_buff **pskb,
			      enum ip_conntrack_info ctinfo,
			      struct nf_conn *ct,
			      const char *dptr)
{
	unsigned int dataoff, matchoff, matchlen;
	char buffer[sizeof("65536")];
	int bufflen;

	dataoff = (*pskb)->nh.iph->ihl*4 + sizeof(struct udphdr);

	/* Get actual SDP lenght */
	if (ct_sip_get_info(ct, dptr, (*pskb)->len - dataoff, &matchoff,
			    &matchlen, POS_SDP_HEADER) > 0) {

		/* since ct_sip_get_info() give us a pointer passing 'v='
		   we need to add 2 bytes in this count. */
		int c_len = (*pskb)->len - dataoff - matchoff + 2;

		/* Now, update SDP length */
		if (ct_sip_get_info(ct, dptr, (*pskb)->len - dataoff, &matchoff,
				    &matchlen, POS_CONTENT) > 0) {

			bufflen = sprintf(buffer, "%u", c_len);
			return nf_nat_mangle_udp_packet(pskb, ct, ctinfo,
							matchoff, matchlen,
							buffer, bufflen);
		}
	}
	return 0;
}

static unsigned int mangle_sdp(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       struct nf_conn *ct,
			       __be32 newip, u_int16_t port,
			       const char *dptr, int av)
{
	char buffer[sizeof("nnn.nnn.nnn.nnn")];
	unsigned int dataoff, bufflen;

	dataoff = (*pskb)->nh.iph->ihl*4 + sizeof(struct udphdr);

	/* Mangle owner and contact info. */
	bufflen = sprintf(buffer, "%u.%u.%u.%u", NIPQUAD(newip));
	if (!mangle_sip_packet(pskb, ctinfo, ct, &dptr, (*pskb)->len - dataoff,
			       buffer, bufflen, POS_OWNER_IP4))
		return 0;

	if (!mangle_sip_packet(pskb, ctinfo, ct, &dptr, (*pskb)->len - dataoff,
			       buffer, bufflen, POS_CONNECTION_IP4))
		return 0;

	/* Mangle media port. */
	bufflen = sprintf(buffer, "%u", port);
#if defined(CONFIG_MIPS_BRCM)
	if (av == (int)POS_MEDIA_AUDIO && !mangle_sip_packet(pskb, ctinfo, ct, &dptr, (*pskb)->len - dataoff,
	                       buffer, bufflen, POS_MEDIA_AUDIO))
		return 0;

	if (av == (int)POS_MEDIA_VIDEO && !mangle_sip_packet(pskb, ctinfo, ct, &dptr, (*pskb)->len - dataoff,
	                       buffer, bufflen, POS_MEDIA_VIDEO))
		return 0;
	/* Mangle ANAT */
	bufflen = sprintf(buffer, "%u.%u.%u.%u %u", NIPQUAD(newip), port);
	mangle_sip_packet(pskb, ctinfo, ct, &dptr, (*pskb)->len - dataoff,
			  buffer, bufflen, POS_ANAT);
#else
	if (!mangle_sip_packet(pskb, ctinfo, ct, &dptr, (*pskb)->len - dataoff,
			       buffer, bufflen, POS_MEDIA))
		return 0;
#endif /* CONFIG_MIPS_BRCM */

	return mangle_content_len(pskb, ctinfo, ct, dptr);
}

static void nf_nat_sdp_expect(struct nf_conn *new,
			      struct nf_conntrack_expect *exp)
{
        struct nf_nat_range range;

        /* This must be a fresh one. */
        BUG_ON(new->status & IPS_NAT_DONE_MASK);

        /* Change src to where new ct comes from */
        range.flags = IP_NAT_RANGE_MAP_IPS;
        range.min_ip = range.max_ip =
                new->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
        nf_nat_setup_info(new, &range, NF_IP_POST_ROUTING);

        /* For DST manip, map ip:port here to where it's expected. */
        range.flags = (IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED);
        range.min = range.max = exp->saved_proto;
        range.min_ip = range.max_ip = exp->saved_ip;
        nf_nat_setup_info(new, &range, NF_IP_PRE_ROUTING);
}


#if defined(CONFIG_MIPS_BRCM)
static int find_fullcone_exp(struct nf_conntrack_expect * sipexp)
{
	struct nf_conntrack_expect * exp;

	list_for_each_entry(exp, &nf_conntrack_expect_list, list) {	
		if (exp->saved_ip == sipexp->saved_ip &&
		    exp->saved_proto.all == sipexp->saved_proto.all &&
		    exp->tuple.dst.protonum == sipexp->tuple.dst.protonum &&
		    exp->tuple.src.u3.ip == 0 &&
		    exp->tuple.src.u.udp.port == 0)
			return exp->saved_proto.udp.port;
	}
	return 0;
}
#endif /* CONFIG_MIPS_BRCM */

/* So, this packet has hit the connection tracking matching code.
   Mangle it, and change the expectation to match the new version. */
static unsigned int ip_nat_sdp(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       struct nf_conntrack_expect *exp,
			       const char *dptr,
			       int rtp, int av)
{
	struct nf_conn *ct = exp->master;
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	__be32 newip;
	u_int16_t port;
	int fullcone;

	DEBUGP("ip_nat_sdp():\n");

	/* Connection will come from reply */
	newip = ct->tuplehash[!dir].tuple.dst.u3.ip;
#if defined(CONFIG_MIPS_BRCM)
	exp->saved_ip = ct->tuplehash[dir].tuple.src.u3.ip;
#endif /* CONFIG_MIPS_BRCM */
	exp->tuple.dst.u3.ip = newip;
	exp->saved_proto.udp.port = exp->tuple.dst.u.udp.port;
	exp->dir = !dir;

	/* When you see the packet, we need to NAT it the same as the
	   this one. */
#if defined(CONFIG_MIPS_BRCM)
	//exp->expectfn = nf_nat_follow_master;
	exp->expectfn = nf_nat_sdp_expect;
#endif /* CONFIG_MIPS_BRCM */


#if defined(CONFIG_MIPS_BRCM)
    /* check if the exp be set by fullcone nat */
    fullcone = find_fullcone_exp(exp);
    if(fullcone) {
		port = fullcone;
	}
	else 
#endif /* CONFIG_MIPS_BRCM */
		{		
		/* Try to get same port: if not, try to change it. */
		for (port = ntohs(exp->saved_proto.udp.port); port != 0; port++) {
			exp->tuple.dst.u.udp.port = htons(port);
			if (nf_conntrack_expect_related(exp) == 0)
				break;
		}

		if (port == 0)
			return NF_DROP;
	}
	
	if (rtp && !mangle_sdp(pskb, ctinfo, ct, newip, port, dptr, av)) {
		nf_conntrack_unexpect_related(exp);
		return NF_DROP;
	}
	return NF_ACCEPT;
}

#if defined(CONFIG_MIPS_BRCM)
static unsigned int ip_nat_reply(struct sk_buff **pskb,
				enum ip_conntrack_info ctinfo,
				struct nf_conn *ct, 
				const char **dptr, unsigned int datalen)
{
    struct addr_map map;
	
	addr_map_init(ct, &map);
   	if(!map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_REQ_URI, &map))
        return NF_DROP;
    if(!map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_VIA, &map))
		return NF_DROP;
    if(!map_sip_addr(pskb, ctinfo, ct, dptr, datalen, POS_CONTACT, &map))
		return NF_DROP;
	return NF_ACCEPT;
}
#endif //CONFIG_MIPS_BRCM

static void __exit nf_nat_sip_fini(void)
{
	rcu_assign_pointer(nf_nat_sip_hook, NULL);
	rcu_assign_pointer(nf_nat_sdp_hook, NULL);
#if defined(CONFIG_MIPS_BRCM)
	rcu_assign_pointer(nf_nat_reply_hook, NULL);
#endif //CONFIG_MIPS_BRCM
	synchronize_rcu();
}

static int __init nf_nat_sip_init(void)
{
	BUG_ON(rcu_dereference(nf_nat_sip_hook));
	BUG_ON(rcu_dereference(nf_nat_sdp_hook));
#if defined(CONFIG_MIPS_BRCM)
	BUG_ON(rcu_dereference(nf_nat_reply_hook));
#endif //CONFIG_MIPS_BRCM
	rcu_assign_pointer(nf_nat_sip_hook, ip_nat_sip);
	rcu_assign_pointer(nf_nat_sdp_hook, ip_nat_sdp);
#if defined(CONFIG_MIPS_BRCM)
	rcu_assign_pointer(nf_nat_reply_hook, ip_nat_reply);
#endif //CONFIG_MIPS_BRCM
	return 0;
}

module_init(nf_nat_sip_init);
module_exit(nf_nat_sip_fini);
