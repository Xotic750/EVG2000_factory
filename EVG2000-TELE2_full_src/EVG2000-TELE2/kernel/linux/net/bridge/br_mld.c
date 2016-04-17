#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include <linux/ip.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>

#include "br_private.h"
#include "br_mld.h"

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)

int br_mld_snooping = 0;
struct proc_dir_entry *br_mld_entry = NULL;

static void mld_addr_conv(unsigned char *in, char * out)
{
    sprintf(out, "%02x%02x%02x%02x%02x%02x", in[0], in[1], in[2], in[3], in[4], in[5]);
}

/* ipv6 address mac address */
static mac_addr upnp_addr = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x0f}};
static mac_addr sys1_addr = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x01}};
static mac_addr sys2_addr = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x02}};
static mac_addr ospf1_addr = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x05}};
static mac_addr ospf2_addr = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x06}};
static mac_addr ripv2_addr = {{0x33, 0x33, 0x5e, 0x00, 0x00, 0x09}};
static mac_addr sys_addr = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

static int mld_control_filter(unsigned char *dest)
{
   if ((!memcmp(dest, &upnp_addr, ETH_ALEN)) ||
       (!memcmp(dest, &sys1_addr, ETH_ALEN)) ||
       (!memcmp(dest, &sys2_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ospf1_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ospf2_addr, ETH_ALEN)) ||
       (!memcmp(dest, &sys_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ripv2_addr, ETH_ALEN)))
      return 0;
   else
      return 1;
}

static void mld_conv_ipv6_to_mac(char *ipa, char *maca)
{
   maca[0] = 0x33;
   maca[1] = 0x33;
   maca[2] = *(ipa+13);
   maca[3] = *(ipa+14);
   maca[4] = *(ipa+15);
   maca[5] = *(ipa+16);

   return;
}

static void mld_query_timeout(unsigned long ptr)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *tmp;
	struct list_head *lh;
	struct net_bridge *br;
    
	br = (struct net_bridge *) ptr;

	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    if (time_after_eq(jiffies, dst->tstamp)) {
		list_del_rcu(&dst->list);
		kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
		
	mod_timer(&br->mld_timer, jiffies + TIMER_CHECK_TIMEOUT*HZ);		
}

static int br_mld_mc_fdb_update(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
	int ret = 0;
	int filt_mode;

        if(mode == SNOOP_IN_ADD)
          filt_mode = MCAST_INCLUDE;
        else
          filt_mode = MCAST_EXCLUDE;
    
	list_for_each_rcu(lh, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) &&
		(filt_mode == dst->src_entry.filt_mode) && 
		(dst->dst == prt) &&
		(!memcmp(&dst->host, host, ETH_ALEN)) && 
	        (!memcmp(src->s6_addr, dst->src_entry.src.s6_addr, sizeof(struct in6_addr)))) {
	             dst->tstamp = jiffies + QUERY_TIMEOUT*HZ;
	             ret = 1;
           }
	}

	return ret;
}

static struct net_br_mld_mc_fdb_entry *br_mld_mc_fdb_get(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
	int filt_mode;
    
        if(mode == SNOOP_IN_CLEAR)
          filt_mode = MCAST_INCLUDE;
        else
          filt_mode = MCAST_EXCLUDE;
          
	list_for_each_rcu(lh, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && 
		(dst->dst == prt) &&
                (filt_mode == dst->src_entry.filt_mode) && 
                (!memcmp(&dst->host, host, ETH_ALEN)) &&
                (!memcmp(dst->src_entry.src.s6_addr, src->s6_addr, sizeof(struct in6_addr)))) {
		    return dst;
	    }
	}
	
	return NULL;
}

int br_mld_mc_fdb_add(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *mc_fdb;

        if(!br || !prt || !dest || !host)
            return 0;

        if((SNOOP_IN_ADD != mode) && (SNOOP_EX_ADD != mode))             
            return 0;

	if (br_mld_mc_fdb_update(br, prt, dest, host, mode, src))
	    return 0;

	mc_fdb = kzalloc(sizeof(struct net_br_mld_mc_fdb_entry), GFP_KERNEL);
	if (!mc_fdb)
	    return ENOMEM;
	memcpy(mc_fdb->addr.addr, dest, ETH_ALEN);
	memcpy(mc_fdb->host.addr, host, ETH_ALEN);
        memcpy(&mc_fdb->src_entry, src, sizeof(struct in6_addr));
	mc_fdb->src_entry.filt_mode = 
                  (mode == SNOOP_IN_ADD) ? MCAST_INCLUDE : MCAST_EXCLUDE;
	mc_fdb->dst = prt;
	mc_fdb->tstamp = jiffies + QUERY_TIMEOUT*HZ;

	spin_lock_bh(&br->mld_mcl_lock);
	list_add_tail_rcu(&mc_fdb->list, &br->mld_mc_list);
	spin_unlock_bh(&br->mld_mcl_lock);

	if (!br->start_timer) {
    	    init_timer(&br->mld_timer);
	    br->mld_timer.expires = jiffies + TIMER_CHECK_TIMEOUT*HZ;
	    br->mld_timer.function = mld_query_timeout;
	    br->mld_timer.data = (unsigned long) br;
	    add_timer(&br->mld_timer);
	    br->start_timer = 1;
	}

	return 1;
}

void br_mld_mc_fdb_cleanup(struct net_bridge *br)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
	struct list_head *tmp;
    
	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    list_del_rcu(&dst->list);
	    kfree(dst);
	}
	spin_unlock_bh(&br->mld_mcl_lock);
}

void br_mld_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
	struct list_head *tmp;

	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && (dst->dst == prt)) {
		list_del_rcu(&dst->list);
		kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mld_mcl_lock);
}

int br_mld_mc_fdb_remove(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *mc_fdb;

        if((SNOOP_IN_CLEAR != mode) && (SNOOP_EX_CLEAR != mode))             
            return 0;
	
	if ((mc_fdb = br_mld_mc_fdb_get(br, prt, dest, host, mode, src))) {
	    spin_lock_bh(&br->mld_mcl_lock);
	    list_del_rcu(&mc_fdb->list);
	    kfree(mc_fdb);
	    spin_unlock_bh(&br->mld_mcl_lock);

	    return 1;
	}
	
	return 0;
}

static struct net_br_mld_mc_fdb_entry *br_mld_mc_fdb_find(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
    
	list_for_each_rcu(lh, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) &&
		(dst->dst == prt) &&
                (!memcmp(&dst->host, host, ETH_ALEN)) &&
                (!memcmp(&dst->src_entry.src, src, sizeof(struct in6_addr)))) {
		    return dst;
	    }
	}
	
	return NULL;
}

void br_mld_process_info(struct net_bridge *br, struct sk_buff *skb)
{
	char destS[16];
	char srcS[16];
	unsigned char tmp[6];
	unsigned char mld_type = 0;
	char *extif = NULL;
	unsigned char *ipv6_addr = NULL;
	unsigned char *src = eth_hdr(skb)->h_source;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

	if (br_mld_snooping && br->mld_proxy) {
	  if (skb->data[6] == IPPROTO_ICMPV6) {
	      mld_type = skb->data[40];
	      ipv6_addr = &skb->data[48];

	  } 
	  else if ((skb->data[6] == IPPROTO_HOPOPTS) &&
                   (skb->data[40] == IPPROTO_ICMPV6)) {
	      mld_type = skb->data[48];
	      ipv6_addr = &skb->data[56];
	  }

	  if((mld_type == ICMPV6_MGM_REPORT) ||
	     (mld_type == ICMPV6_MGM_REDUCTION) || 
	     (mld_type == ICMPV6_MLD2_REPORT)) {

	    if (mld_type == ICMPV6_MGM_REDUCTION) {
			
              mld_conv_ipv6_to_mac(ipv6_addr, tmp);

	      mld_addr_conv(tmp, destS);
	    }
	    else {
	      mld_addr_conv(dest, destS);
            }
	    mld_addr_conv(src, srcS);

	    extif = kzalloc(BCM_SNOOPING_BUFSZ, GFP_ATOMIC);

	    if(extif) {
	      sprintf(extif, "%s %s %s/%s", 
                        br->dev->name, p->dev->name, destS, srcS);
	      skb->extif = extif;
	    }
          }
	}

	return;
} /* br_process_mld_info */

static void br_mld_process_v2(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest, struct mld2_report *report)
{
  struct mld2_grec *grec;
  int i;
  struct in6_addr *src;
  struct in6_addr asrc;
  unsigned char *mldv2_mcast;
  int num_src;
  unsigned char tmp[6];
  struct net_br_mld_mc_fdb_entry *mc_fdb;

  if(report) {
    grec = &report->grec[0];
    for(i = 0; i < report->ngrec; i++) {
      mldv2_mcast = &grec->grec_mca.s6_addr[0];
      mld_conv_ipv6_to_mac(mldv2_mcast, tmp);
      switch(grec->grec_type) {
        case MLD2_MODE_IS_INCLUDE:
        case MLD2_CHANGE_TO_INCLUDE:
        case MLD2_ALLOW_NEW_SOURCES:
          for(num_src = 0; num_src < grec->grec_nsrcs; num_src++) {
            src = &grec->grec_src[num_src];
            mc_fdb = br_mld_mc_fdb_find(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, src);
            if((NULL != mc_fdb) && 
               (mc_fdb->src_entry.filt_mode == MCAST_EXCLUDE)) {
              br_mld_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_CLEAR, src);
            }
            else {
              br_mld_mc_fdb_add(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_IN_ADD, src);
            }
          }

          if(0 == grec->grec_nsrcs) {
            memset(&asrc, 0, sizeof(struct in6_addr));
            br_mld_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_CLEAR, &asrc);
          }
         break;
       
         case MLD2_MODE_IS_EXCLUDE:
         case MLD2_CHANGE_TO_EXCLUDE:
         case MLD2_BLOCK_OLD_SOURCES:
          for(num_src = 0; num_src < grec->grec_nsrcs; num_src++) {
            src = &grec->grec_src[num_src];
            mc_fdb = br_mld_mc_fdb_find(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, src);
            if((NULL != mc_fdb) && 
               (mc_fdb->src_entry.filt_mode == MCAST_INCLUDE)) {
              br_mld_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_IN_CLEAR, src);
            }
            else {
              br_mld_mc_fdb_add(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_ADD, src);
            }
          }

          if(0 == grec->grec_nsrcs) {
            memset(&asrc, 0, sizeof(struct in6_addr));
            br_mld_mc_fdb_add(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_ADD, &asrc);
          }
        break;
      }
      grec = (struct mld2_grec *)((char *)grec + MLDV2_GRP_REC_SIZE(grec));
    }
  }
  return;
}

int br_mld_mc_forward(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest,int forward, int clone)
{
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
	int status = 0;
	struct sk_buff *skb2;
	struct net_bridge_port *p;
	unsigned char tmp[6];
	struct mld2_report *report;
	struct in6_addr src;
        unsigned char *mld_type = NULL;
	unsigned char *ipv6_addr = NULL;
	struct ipv6hdr *pipv6 = skb->nh.ipv6h;

	if (!br_mld_snooping)
		return 0;

	if ((br_mld_snooping == SNOOPING_BLOCKING_MODE) && mld_control_filter(dest))
	    status = 1;

	if(!br->mld_proxy) {
	   if (skb->data[6] == IPPROTO_ICMPV6) {
              mld_type = &skb->data[40];
	      ipv6_addr = &skb->data[48];
	   } 
	   else if ((skb->data[6] == IPPROTO_HOPOPTS) &&
                   (skb->data[40] == IPPROTO_ICMPV6)) {
	      mld_type = &skb->data[48];
	      ipv6_addr = &skb->data[56];
	   }

	    if((mld_type != NULL) && ((*mld_type == ICMPV6_MGM_REPORT) ||
	     (*mld_type == ICMPV6_MGM_REDUCTION) || 
	     (*mld_type == ICMPV6_MLD2_REPORT))) {
		if ((*mld_type == ICMPV6_MGM_REPORT) &&
		    (skb->protocol == __constant_htons(ETH_P_IPV6))) {
	            memset(&src, 0, sizeof(struct in6_addr));
		    br_mld_mc_fdb_add(br, skb->dev->br_port, dest, eth_hdr(skb)->h_source, SNOOP_EX_ADD, &src);
                }
                else if((*mld_type == ICMPV6_MLD2_REPORT) &&
                        (skb->protocol == __constant_htons(ETH_P_IPV6))) {
                      report = (struct mld2_report *)mld_type;
                    if(report) {
                      br_mld_process_v2(br, skb, dest, report);
                    }
                }
		else if (*mld_type == ICMPV6_MGM_REDUCTION) {
		    mld_conv_ipv6_to_mac(ipv6_addr, tmp);
	            memset(&src, 0, sizeof(struct in6_addr));
		    br_mld_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_CLEAR, &src);
		}
		else
		    ;
	    }
	    return 0;
	}

	list_for_each_rcu(lh, &br->mld_mc_list) {
	    dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
	    if (!memcmp(&dst->addr, dest, ETH_ALEN)) {
              if((dst->src_entry.filt_mode == MCAST_INCLUDE) && 
                 (!memcmp(&pipv6->saddr, &dst->src_entry.src, sizeof(struct in6_addr)))) {

		if (!dst->dst->dirty) {
		    skb2 = skb_clone(skb, GFP_ATOMIC);
		    if (forward)
			br_forward(dst->dst, skb2);
		    else
			br_deliver(dst->dst, skb2);
		}
		dst->dst->dirty = 1;
		status = 1;
              }
              else if(dst->src_entry.filt_mode == MCAST_EXCLUDE) {
                if((0 == dst->src_entry.src.s6_addr[0]) ||
                   (!memcmp(&pipv6->saddr, &dst->src_entry.src, sizeof(struct in6_addr)))) {

		  if (!dst->dst->dirty) {
		    skb2 = skb_clone(skb, GFP_ATOMIC);
		    if (forward)
			br_forward(dst->dst, skb2);
		    else
			br_deliver(dst->dst, skb2);
		  }
		  dst->dst->dirty = 1;
		  status = 1;
                }
              }
	    }
	}
	if (status) {
	    list_for_each_entry_rcu(p, &br->port_list, list) {
		p->dirty = 0;
	  }
	}

	if ((!forward) && (status))
	kfree_skb(skb);

	return status;
}

int br_mld_set_port_snooping(struct net_bridge_port *p,  void __user * userbuf)
{
    unsigned char tmp[26];
    struct in6_addr src;

    if (copy_from_user(tmp, userbuf, sizeof(tmp)))
		return -EFAULT;
    
    memcpy(&src, tmp+13, sizeof(struct in6_addr));
    br_mld_mc_fdb_add(p->br, p, tmp, tmp+6, tmp[12], &src);
    return 0;
}

int br_mld_clear_port_snooping(struct net_bridge_port *p,  void __user * userbuf)
{
    unsigned char tmp[26];
    unsigned char all[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    struct in6_addr src;

    if (copy_from_user(tmp, userbuf, sizeof(tmp)))
		return -EFAULT;

    p->br->igmp_proxy = 1;
    if (!memcmp(tmp+6, all, 6)) {
	br_mld_mc_fdb_remove_grp(p->br, p, tmp);
    }
    else {
	memcpy(&src, tmp+13, sizeof(struct in6_addr));
	br_mld_mc_fdb_remove(p->br, p, tmp, tmp+6, tmp[12], &src);
    }
    return 1;
}

static void *snoop_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net_device *dev;
	loff_t offs = 0;

	rtnl_lock();
	for(dev = dev_base; dev; dev = dev->next) {
		if ((dev->priv_flags & IFF_EBRIDGE) &&
                    (*pos == offs)) { 
			return dev;
		}
	}
	++offs;
	return NULL;
}

static void *snoop_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net_device *dev = v;

	++*pos;
	
	for(dev = dev->next; dev; dev = dev->next) {
		if(dev->priv_flags & IFF_EBRIDGE)
			return dev;
	}
	return NULL;
}

static int snoop_seq_show(struct seq_file *seq, void *v)
{
	struct net_device *dev = v;
	struct net_br_mld_mc_fdb_entry *dst;
	struct list_head *lh;
	struct net_bridge *br = netdev_priv(dev);
	

	seq_printf(seq, "bridge	device	group		   reporter          mode  source timeout\n");

	list_for_each_rcu(lh, &br->mld_mc_list) {
		dst = (struct net_br_mld_mc_fdb_entry *) list_entry(lh, struct net_br_mld_mc_fdb_entry, list);
		seq_printf(seq, "%s %6s    ", br->dev->name, dst->dst->dev->name);
		seq_printf(seq, "%02x:%02x:%02x:%02x:%02x:%02x   ", 
			dst->addr.addr[0], dst->addr.addr[1], 
			dst->addr.addr[2], dst->addr.addr[3], 
			dst->addr.addr[4], dst->addr.addr[5]);

		seq_printf(seq, "%02x:%02x:%02x:%02x:%02x:%02x   ", 
			dst->host.addr[0], dst->host.addr[1], 
			dst->host.addr[2], dst->host.addr[3], 
			dst->host.addr[4], dst->host.addr[5]);

		seq_printf(seq, "%2s  %04x:%04x:%04x:%04x   %d\n", 
			(dst->src_entry.filt_mode == MCAST_EXCLUDE) ? 
			"EX" : "IN",
			dst->src_entry.src.s6_addr32[0], 
			dst->src_entry.src.s6_addr32[1], 
			dst->src_entry.src.s6_addr32[2], 
			dst->src_entry.src.s6_addr32[3], 
			(int) (dst->tstamp - jiffies)/HZ);
	}

	return 0;
}

static void snoop_seq_stop(struct seq_file *seq, void *v)
{
	rtnl_unlock();
}

static struct seq_operations snoop_seq_ops = {
	.start = snoop_seq_start,
	.next  = snoop_seq_next,
	.stop  = snoop_seq_stop,
	.show  = snoop_seq_show,
};

static int snoop_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &snoop_seq_ops);
}

static struct file_operations br_mld_snoop_proc_fops = {
	.owner = THIS_MODULE,
	.open  = snoop_seq_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void br_mld_snooping_init(void)
{
	br_mld_entry = create_proc_entry("net/mld_snooping", 0, NULL);

	if(br_mld_entry) {
		br_mld_entry->proc_fops = &br_mld_snoop_proc_fops;
	}
}

#endif
