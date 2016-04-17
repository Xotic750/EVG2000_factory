#ifndef _BR_MLD_H
#define _BR_MLD_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/igmp.h>
#include <linux/in.h>

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)

#define SNOOPING_BLOCKING_MODE 2

#define TIMER_CHECK_TIMEOUT 10
#define QUERY_TIMEOUT 130


#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
#define BRCTL_MLD_SET_PORT_SNOOPING 26
#define BRCTL_MLD_CLEAR_PORT_SNOOPING 27
#define BRCTL_MLD_ENABLE_SNOOPING 28
#define BRCTL_MLD_ENABLE_PROXY_MODE 29
#endif

struct mld2_grec {
	__u8		grec_type;
	__u8		grec_auxwords;
	__be16		grec_nsrcs;
	struct in6_addr	grec_mca;
	struct in6_addr	grec_src[0];
};

struct mld2_report {
	__u8	type;
	__u8	resv1;
	__sum16	csum;
	__be16	resv2;
	__be16	ngrec;
	struct mld2_grec grec[0];
};

#define MLDV2_GRP_REC_SIZE(x)  (sizeof(struct mld2_grec) + \
                       (sizeof(struct in6_addr) * ((struct mld2_grec *)x)->grec_nsrcs))

struct net_br_mld_mc_src_entry
{
	struct in6_addr		src;
	unsigned long		tstamp;
        int			filt_mode;
};

struct net_br_mld_mc_fdb_entry
{
	struct net_bridge_port		*dst;
	mac_addr			addr;
	mac_addr			host;
	struct net_br_mld_mc_src_entry src_entry;
	unsigned char			is_local;
	unsigned char			is_static;
	unsigned long			tstamp;
	struct list_head 		list;
};



extern int br_mld_snooping;
extern int br_mld_mc_forward(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest,int forward, int clone);
void br_mld_process_info(struct net_bridge *br, struct sk_buff *skb);
void br_mld_delbr_cleanup(struct net_bridge *br);

extern int br_mld_mc_fdb_add(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in6_addr *src);
extern void br_mld_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest);
extern void br_mld_mc_fdb_cleanup(struct net_bridge *br);
extern int br_mld_mc_fdb_remove(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in6_addr *src);
void br_mld_snooping_init(void);
extern int br_mld_set_port_snooping(struct net_bridge_port *p,  void __user * userbuf);
extern int br_mld_clear_port_snooping(struct net_bridge_port *p,  void __user * userbuf);
#endif
#endif /* _BR_MLD_H */
