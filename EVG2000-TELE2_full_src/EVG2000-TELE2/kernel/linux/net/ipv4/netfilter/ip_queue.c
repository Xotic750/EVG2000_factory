/*
 * This is a module which is used for queueing IPv4 packets and
 * communicating with userspace via netlink.
 *
 * (C) 2000-2002 James Morris <jmorris@intercode.com.au>
 * (C) 2003-2005 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 2000-03-27: Simplified code (thanks to Andi Kleen for clues).
 * 2000-05-20: Fixed notifier problems (following Miguel Freitas' report).
 * 2000-06-19: Fixed so nfmark is copied to metadata (reported by Sebastian
 *             Zander).
 * 2000-08-01: Added Nick Williams' MAC support.
 * 2002-06-25: Code cleanup.
 * 2005-01-10: Added /proc counter for dropped packets; fixed so
 *             packets aren't delivered to user space if they're going
 *             to be dropped.
 * 2005-05-26: local_bh_{disable,enable} around nf_reinject (Harald Welte)
 *
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/notifier.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4/ip_queue.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/security.h>
#include <linux/mutex.h>
#include <net/sock.h>
#include <net/route.h>

#define IPQ_QMAX_DEFAULT 1024
#define IPQ_PROC_FS_NAME "ip_queue"
#define NET_IPQ_QMAX 2088
#define NET_IPQ_QMAX_NAME "ip_queue_maxlen"

struct ipq_queue_entry {
	struct list_head list;
	struct nf_info *info;
	struct sk_buff *skb;
};

typedef int (*ipq_cmpfn)(struct ipq_queue_entry *, unsigned long);

static unsigned char copy_mode __read_mostly = IPQ_COPY_NONE;
static unsigned int queue_maxlen __read_mostly = IPQ_QMAX_DEFAULT;
static DEFINE_RWLOCK(queue_lock);
static int peer_pid __read_mostly;
static unsigned int copy_range __read_mostly;
static unsigned int queue_total;
static unsigned int queue_dropped = 0;
static unsigned int queue_user_dropped = 0;
static struct sock *ipqnl __read_mostly;
static LIST_HEAD(queue_list);
static DEFINE_MUTEX(ipqnl_mutex);

static void
ipq_issue_verdict(struct ipq_queue_entry *entry, int verdict)
{
	/* TCP input path (and probably other bits) assume to be called
	 * from softirq context, not from syscall, like ipq_issue_verdict is
	 * called.  TCP input path deadlocks with locks taken from timer
	 * softirq, e.g.  We therefore emulate this by local_bh_disable() */

	local_bh_disable();
	nf_reinject(entry->skb, entry->info, verdict);
	local_bh_enable();

	kfree(entry);
}

static inline void
__ipq_enqueue_entry(struct ipq_queue_entry *entry)
{
       list_add(&entry->list, &queue_list);
       queue_total++;
}

/*
 * Find and return a queued entry matched by cmpfn, or return the last
 * entry if cmpfn is NULL.
 */
static inline struct ipq_queue_entry *
__ipq_find_entry(ipq_cmpfn cmpfn, unsigned long data)
{
	struct list_head *p;

	list_for_each_prev(p, &queue_list) {
		struct ipq_queue_entry *entry = (struct ipq_queue_entry *)p;

		if (!cmpfn || cmpfn(entry, data))
			return entry;
	}
	return NULL;
}

static inline void
__ipq_dequeue_entry(struct ipq_queue_entry *entry)
{
	list_del(&entry->list);
	queue_total--;
}

static inline struct ipq_queue_entry *
__ipq_find_dequeue_entry(ipq_cmpfn cmpfn, unsigned long data)
{
	struct ipq_queue_entry *entry;

	entry = __ipq_find_entry(cmpfn, data);
	if (entry == NULL)
		return NULL;

	__ipq_dequeue_entry(entry);
	return entry;
}


static inline void
__ipq_flush(int verdict)
{
	struct ipq_queue_entry *entry;

	while ((entry = __ipq_find_dequeue_entry(NULL, 0)))
		ipq_issue_verdict(entry, verdict);
}

static inline int
__ipq_set_mode(unsigned char mode, unsigned int range)
{
	int status = 0;

	switch(mode) {
	case IPQ_COPY_NONE:
	case IPQ_COPY_META:
		copy_mode = mode;
		copy_range = 0;
		break;

	case IPQ_COPY_PACKET:
		copy_mode = mode;
		copy_range = range;
		if (copy_range > 0xFFFF)
			copy_range = 0xFFFF;
		break;

	default:
		status = -EINVAL;

	}
	return status;
}

static inline void
__ipq_reset(void)
{
	peer_pid = 0;
	net_disable_timestamp();
	__ipq_set_mode(IPQ_COPY_NONE, 0);
	__ipq_flush(NF_DROP);
}

static struct ipq_queue_entry *
ipq_find_dequeue_entry(ipq_cmpfn cmpfn, unsigned long data)
{
	struct ipq_queue_entry *entry;

	write_lock_bh(&queue_lock);
	entry = __ipq_find_dequeue_entry(cmpfn, data);
	write_unlock_bh(&queue_lock);
	return entry;
}

static void
ipq_flush(int verdict)
{
	write_lock_bh(&queue_lock);
	__ipq_flush(verdict);
	write_unlock_bh(&queue_lock);
}

static struct sk_buff *
ipq_build_packet_message(struct ipq_queue_entry *entry, int *errp)
{
	unsigned char *old_tail;
	size_t size = 0;
	size_t data_len = 0;
	struct sk_buff *skb;
	struct ipq_packet_msg *pmsg;
	struct nlmsghdr *nlh;

	read_lock_bh(&queue_lock);

	switch (copy_mode) {
	case IPQ_COPY_META:
	case IPQ_COPY_NONE:
		size = NLMSG_SPACE(sizeof(*pmsg));
		data_len = 0;
		break;

	case IPQ_COPY_PACKET:
		if ((entry->skb->ip_summed == CHECKSUM_PARTIAL ||
		     entry->skb->ip_summed == CHECKSUM_COMPLETE) &&
		    (*errp = skb_checksum_help(entry->skb))) {
			read_unlock_bh(&queue_lock);
			return NULL;
		}
		if (copy_range == 0 || copy_range > entry->skb->len)
			data_len = entry->skb->len;
		else
			data_len = copy_range;

		size = NLMSG_SPACE(sizeof(*pmsg) + data_len);
		break;

	default:
		*errp = -EINVAL;
		read_unlock_bh(&queue_lock);
		return NULL;
	}

	read_unlock_bh(&queue_lock);

	skb = alloc_skb(size, GFP_ATOMIC);
	if (!skb)
		goto nlmsg_failure;

	old_tail= skb->tail;
	nlh = NLMSG_PUT(skb, 0, 0, IPQM_PACKET, size - sizeof(*nlh));
	pmsg = NLMSG_DATA(nlh);
	memset(pmsg, 0, sizeof(*pmsg));

	pmsg->packet_id       = (unsigned long )entry;
	pmsg->data_len        = data_len;
	pmsg->timestamp_sec   = entry->skb->tstamp.off_sec;
	pmsg->timestamp_usec  = entry->skb->tstamp.off_usec;
	pmsg->mark            = entry->skb->mark;
	pmsg->hook            = entry->info->hook;
	pmsg->hw_protocol     = entry->skb->protocol;

	if (entry->info->indev)
		strcpy(pmsg->indev_name, entry->info->indev->name);
	else
		pmsg->indev_name[0] = '\0';

	if (entry->info->outdev)
		strcpy(pmsg->outdev_name, entry->info->outdev->name);
	else
		pmsg->outdev_name[0] = '\0';

	if (entry->info->indev && entry->skb->dev) {
		pmsg->hw_type = entry->skb->dev->type;
		if (entry->skb->dev->hard_header_parse)
			pmsg->hw_addrlen =
				entry->skb->dev->hard_header_parse(entry->skb,
								   pmsg->hw_addr);
	}

	if (data_len)
		if (skb_copy_bits(entry->skb, 0, pmsg->payload, data_len))
			BUG();

	nlh->nlmsg_len = skb->tail - old_tail;
	return skb;

nlmsg_failure:
	if (skb)
		kfree_skb(skb);
	*errp = -EINVAL;
	printk(KERN_ERR "ip_queue: error creating packet message\n");
	return NULL;
}

static int
ipq_enqueue_packet(struct sk_buff *skb, struct nf_info *info,
		   unsigned int queuenum, void *data)
{
	int status = -EINVAL;
	struct sk_buff *nskb;
	struct ipq_queue_entry *entry;

#if defined(CONFIG_MIPS_BRCM)
	if (copy_mode == IPQ_COPY_NONE && !skb->ipvs_property)
	#else
	if (copy_mode == IPQ_COPY_NONE)
#endif /* CONFIG_MIPS_BRCM */
		return -EAGAIN;

	entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
	if (entry == NULL) {
		printk(KERN_ERR "ip_queue: OOM in ipq_enqueue_packet()\n");
		return -ENOMEM;
	}
#if defined(CONFIG_MIPS_BRCM) && 0
	        skb->nat_cache_add = NULL;
#endif

	entry->info = info;
	entry->skb = skb;

#if defined(CONFIG_MIPS_BRCM)
	if (skb->ipvs_property) {
		write_lock_bh(&queue_lock);
		__ipq_enqueue_entry(entry);
		write_unlock_bh(&queue_lock);
		return 1;
	}
#endif /* CONFIG_MIPS_BRCM */

	nskb = ipq_build_packet_message(entry, &status);
	if (nskb == NULL)
		goto err_out_free;

	write_lock_bh(&queue_lock);

	if (!peer_pid)
		goto err_out_free_nskb;

	if (queue_total >= queue_maxlen) {
		queue_dropped++;
		status = -ENOSPC;
		if (net_ratelimit())
			  printk (KERN_WARNING "ip_queue: full at %d entries, "
				  "dropping packets(s). Dropped: %d\n", queue_total,
				  queue_dropped);
		goto err_out_free_nskb;
	}

	/* netlink_unicast will either free the nskb or attach it to a socket */
	status = netlink_unicast(ipqnl, nskb, peer_pid, MSG_DONTWAIT);
	if (status < 0) {
		queue_user_dropped++;
		goto err_out_unlock;
	}

	__ipq_enqueue_entry(entry);

	write_unlock_bh(&queue_lock);
	return status;

err_out_free_nskb:
	kfree_skb(nskb);

err_out_unlock:
	write_unlock_bh(&queue_lock);

err_out_free:
	kfree(entry);
	return status;
}

static int
ipq_mangle_ipv4(ipq_verdict_msg_t *v, struct ipq_queue_entry *e)
{
	int diff;
	struct iphdr *user_iph = (struct iphdr *)v->payload;

	if (v->data_len < sizeof(*user_iph))
		return 0;
	diff = v->data_len - e->skb->len;
	if (diff < 0) {
		if (pskb_trim(e->skb, v->data_len))
			return -ENOMEM;
	} else if (diff > 0) {
		if (v->data_len > 0xFFFF)
			return -EINVAL;
		if (diff > skb_tailroom(e->skb)) {
			struct sk_buff *newskb;

			newskb = skb_copy_expand(e->skb,
						 skb_headroom(e->skb),
						 diff,
						 GFP_ATOMIC);
			if (newskb == NULL) {
				printk(KERN_WARNING "ip_queue: OOM "
				      "in mangle, dropping packet\n");
				return -ENOMEM;
			}
			if (e->skb->sk)
				skb_set_owner_w(newskb, e->skb->sk);
			kfree_skb(e->skb);
			e->skb = newskb;
		}
		skb_put(e->skb, diff);
	}
	if (!skb_make_writable(&e->skb, v->data_len))
		return -ENOMEM;
	memcpy(e->skb->data, v->payload, v->data_len);
	e->skb->ip_summed = CHECKSUM_NONE;

	return 0;
}

static inline int
id_cmp(struct ipq_queue_entry *e, unsigned long id)
{
	return (id == (unsigned long )e);
}

static int
ipq_set_verdict(struct ipq_verdict_msg *vmsg, unsigned int len)
{
	struct ipq_queue_entry *entry;

	if (vmsg->value > NF_MAX_VERDICT)
		return -EINVAL;

	entry = ipq_find_dequeue_entry(id_cmp, vmsg->id);
	if (entry == NULL)
		return -ENOENT;
	else {
		int verdict = vmsg->value;

		if (vmsg->data_len && vmsg->data_len == len)
			if (ipq_mangle_ipv4(vmsg, entry) < 0)
				verdict = NF_DROP;

		ipq_issue_verdict(entry, verdict);
		return 0;
	}
}

static int
ipq_set_mode(unsigned char mode, unsigned int range)
{
	int status;

	write_lock_bh(&queue_lock);
	status = __ipq_set_mode(mode, range);
	write_unlock_bh(&queue_lock);
	return status;
}

static int
ipq_receive_peer(struct ipq_peer_msg *pmsg,
		 unsigned char type, unsigned int len)
{
	int status = 0;

	if (len < sizeof(*pmsg))
		return -EINVAL;

	switch (type) {
	case IPQM_MODE:
		status = ipq_set_mode(pmsg->msg.mode.value,
				      pmsg->msg.mode.range);
		break;

	case IPQM_VERDICT:
		if (pmsg->msg.verdict.value > NF_MAX_VERDICT)
			status = -EINVAL;
		else
			status = ipq_set_verdict(&pmsg->msg.verdict,
						 len - sizeof(*pmsg));
			break;
	default:
		status = -EINVAL;
	}
	return status;
}

static int
dev_cmp(struct ipq_queue_entry *entry, unsigned long ifindex)
{
	if (entry->info->indev)
		if (entry->info->indev->ifindex == ifindex)
			return 1;
	if (entry->info->outdev)
		if (entry->info->outdev->ifindex == ifindex)
			return 1;
#ifdef CONFIG_BRIDGE_NETFILTER
	if (entry->skb->nf_bridge) {
		if (entry->skb->nf_bridge->physindev &&
		    entry->skb->nf_bridge->physindev->ifindex == ifindex)
			return 1;
		if (entry->skb->nf_bridge->physoutdev &&
		    entry->skb->nf_bridge->physoutdev->ifindex == ifindex)
			return 1;
	}
#endif
	return 0;
}

static void
ipq_dev_drop(int ifindex)
{
	struct ipq_queue_entry *entry;

	while ((entry = ipq_find_dequeue_entry(dev_cmp, ifindex)) != NULL)
		ipq_issue_verdict(entry, NF_DROP);
}

#define RCV_SKB_FAIL(err) do { netlink_ack(skb, nlh, (err)); return; } while (0)

static inline void
ipq_rcv_skb(struct sk_buff *skb)
{
	int status, type, pid, flags, nlmsglen, skblen;
	struct nlmsghdr *nlh;

	skblen = skb->len;
	if (skblen < sizeof(*nlh))
		return;

	nlh = (struct nlmsghdr *)skb->data;
	nlmsglen = nlh->nlmsg_len;
	if (nlmsglen < sizeof(*nlh) || skblen < nlmsglen)
		return;

	pid = nlh->nlmsg_pid;
	flags = nlh->nlmsg_flags;

	if(pid <= 0 || !(flags & NLM_F_REQUEST) || flags & NLM_F_MULTI)
		RCV_SKB_FAIL(-EINVAL);

	if (flags & MSG_TRUNC)
		RCV_SKB_FAIL(-ECOMM);

	type = nlh->nlmsg_type;
	if (type < NLMSG_NOOP || type >= IPQM_MAX)
		RCV_SKB_FAIL(-EINVAL);

	if (type <= IPQM_BASE)
		return;

	if (security_netlink_recv(skb, CAP_NET_ADMIN))
		RCV_SKB_FAIL(-EPERM);

	write_lock_bh(&queue_lock);

	if (peer_pid) {
		if (peer_pid != pid) {
			write_unlock_bh(&queue_lock);
			RCV_SKB_FAIL(-EBUSY);
		}
	} else {
		net_enable_timestamp();
		peer_pid = pid;
	}

	write_unlock_bh(&queue_lock);

	status = ipq_receive_peer(NLMSG_DATA(nlh), type,
				  nlmsglen - NLMSG_LENGTH(0));
	if (status < 0)
		RCV_SKB_FAIL(status);

	if (flags & NLM_F_ACK)
		netlink_ack(skb, nlh, 0);
	return;
}

static void
ipq_rcv_sk(struct sock *sk, int len)
{
	struct sk_buff *skb;
	unsigned int qlen;

	mutex_lock(&ipqnl_mutex);

	for (qlen = skb_queue_len(&sk->sk_receive_queue); qlen; qlen--) {
		skb = skb_dequeue(&sk->sk_receive_queue);
		ipq_rcv_skb(skb);
		kfree_skb(skb);
	}

	mutex_unlock(&ipqnl_mutex);
}

static int
ipq_rcv_dev_event(struct notifier_block *this,
		  unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;

	/* Drop any packets associated with the downed device */
	if (event == NETDEV_DOWN)
		ipq_dev_drop(dev->ifindex);
	return NOTIFY_DONE;
}

#if defined(CONFIG_MIPS_BRCM)
#if 1
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif /* CONFIG_MIPS_BRCM */

/* Dynahelper tracker status */
#define DH_STAT_UNLOADED 0
#define DH_STAT_LOADING 1
#define DH_STAT_LOADED 2
#define DH_STAT_RUNNING 3

struct dh_tracker {
	struct list_head list;
	char proto[DYNAHELPER_MAXPROTONAMELEN + 1];
	unsigned long timeout;
	int stat;
	u_int32_t refcount;
	struct module *module;
	struct timer_list timer;
};

extern void (*dynahelper_track)(struct module * m);
extern void (*dynahelper_untrack)(struct module * m);
extern void (*dynahelper_ref)(struct module * m);
extern void (*dynahelper_unref)(struct module * m);

static rwlock_t dh_lock = RW_LOCK_UNLOCKED;
static LIST_HEAD(dh_trackers);
static int dh_pid;
static struct sock *dhnl;
static DECLARE_MUTEX(dhnl_sem);

/****************************************************************************/
static void dh_send_msg(struct dh_tracker *tracker, int type)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct ipq_packet_msg *pmsg;

	if (!dhnl)
		return;

	skb = alloc_skb(NLMSG_SPACE(sizeof(*pmsg)), GFP_ATOMIC);
	if (!skb) {
		printk(KERN_ERR "dh_send_msg: alloc_skb() error\n");
		return;
	}
	nlh = __nlmsg_put(skb, 0, 0, type, sizeof(*pmsg), 0);
	pmsg = NLMSG_DATA(nlh);
	strcpy(pmsg->indev_name, tracker->proto);
	netlink_unicast(dhnl, skb, dh_pid, MSG_DONTWAIT);
}

/****************************************************************************/
static inline void dh_load_helper(struct dh_tracker *tracker)
{
	DEBUGP("dh_load_helper: load helper %s\n", tracker->proto);
	dh_send_msg(tracker, IPQM_DYNAHELPER_LOAD);
}

/****************************************************************************/
static inline void dh_unload_helper(struct dh_tracker *tracker)
{
	DEBUGP("dh_unload_helper: unload helper %s\n", tracker->proto);
	dh_send_msg(tracker, IPQM_DYNAHELPER_UNLOAD);
}

/****************************************************************************/
static void dh_start_timer(struct dh_tracker *tracker, int timeout)
{
	if (timeout == 0) {
		if ((timeout = tracker->timeout * HZ) == 0)
			return;
	}

	mod_timer(&tracker->timer, jiffies + timeout);
	DEBUGP("dh_start_timer: helper %s timer started\n", tracker->proto);
}

/****************************************************************************/
static void dh_stop_timer(struct dh_tracker *tracker)
{
	if (del_timer(&tracker->timer)) {
		DEBUGP("dh_stop_timer: helper %s timer stopped\n",
		       tracker->proto);
	}
}

/****************************************************************************/
static void dh_release_packets(unsigned long mark);
static void dh_timer_handler(unsigned long ul_tracker)
{
	struct dh_tracker *tracker = (void *) ul_tracker;

	/* Prevent dh_target from queuing more packets */
	write_lock_bh(&queue_lock);
	write_lock_bh(&dh_lock);

	switch(tracker->stat) {
	case DH_STAT_LOADED:
		tracker->stat = DH_STAT_RUNNING;
		if (tracker->refcount == 0)
			dh_start_timer(tracker, 0);
		break;
	case DH_STAT_LOADING:
	case DH_STAT_RUNNING:
		DEBUGP("dh_timer_handler: helper %s %stimed out\n",
		       tracker->proto,
		       tracker->stat == DH_STAT_LOADING? "loading " : "");
		dh_unload_helper(tracker);
		tracker->stat = DH_STAT_UNLOADED;
		tracker->module = NULL;
		tracker->refcount = 0;
		break;
	}

	write_unlock_bh(&dh_lock);

	DEBUGP("dh_timer_handler: release packets for helper %s\n",
	       tracker->proto);
	dh_release_packets(ul_tracker);

	write_unlock_bh(&queue_lock);
}

/****************************************************************************/
static struct dh_tracker *dh_create_tracker(struct xt_dynahelper_info *info)
{
	struct dh_tracker *tracker;

	tracker = kmalloc(sizeof(struct dh_tracker), GFP_ATOMIC);
	if (!tracker) {
		if (net_ratelimit())
			printk(KERN_ERR "xt_DYNAHELPER: OOM\n");
		return NULL;
	}
	memset(tracker, 0, sizeof(struct dh_tracker));
	strcpy(tracker->proto, info->proto);
	setup_timer(&tracker->timer, dh_timer_handler, (unsigned long)tracker);
	list_add(&tracker->list, &dh_trackers);
	DEBUGP("xt_DYNAHELPER: tracker for helper %s created\n",
	       tracker->proto);

	return tracker;
}

/****************************************************************************/
static void dh_destroy_trackers(void)
{
	struct dh_tracker *tracker;
	struct dh_tracker *tmp;

	list_for_each_entry_safe(tracker, tmp, &dh_trackers, list) {
		list_del(&tracker->list);
		del_timer(&tracker->timer);
		kfree(tracker);
	}
}

/****************************************************************************/
static inline struct dh_tracker *dh_find_tracker_by_proto(char *proto)
{
	struct dh_tracker *tracker;

	list_for_each_entry(tracker, &dh_trackers, list) {
		if (strcmp(tracker->proto, proto) == 0)
			return tracker;
	}
	return NULL;
}

/****************************************************************************/
static inline struct dh_tracker *dh_find_tracker_by_mark(unsigned long mark)
{
	struct dh_tracker *tracker;

	list_for_each_entry(tracker, &dh_trackers, list) {
		if (tracker == (struct dh_tracker*)mark)
			return tracker;
	}
	return NULL;
}

/****************************************************************************/
static inline struct dh_tracker *dh_find_tracker_by_module(struct module *m)
{
	struct dh_tracker *tracker;

	list_for_each_entry(tracker, &dh_trackers, list) {
		if (tracker->stat != DH_STAT_UNLOADED && tracker->module == m)
			return tracker;
	}
	return NULL;
}

/****************************************************************************/
static unsigned int dh_target(struct sk_buff **pskb,
			      const struct net_device *in,
			      const struct net_device *out,
			      unsigned int hooknum,
			      const struct xt_target *target,
			      const void *targinfo)
{
	struct xt_dynahelper_info *info =
	    (struct xt_dynahelper_info *) targinfo;
	struct dh_tracker *tracker = info->tracker;

	DEBUGP("xt_DYNAHELPER: target: tracker=%p, timeout=%lu, proto=%s\n",
	       tracker, info->timeout, info->proto);

	/* Other threads may be releasing the queue */
	write_lock_bh(&queue_lock);
	write_lock_bh(&dh_lock);

	/* Is the user space daemon runing? */
	if (!dh_pid) {
		DEBUGP("xt_DYNAHELPER: dynahelper not running\n");
		goto pass_it;
	}

	/* Lookup by proto name */
	if (!tracker) {
		tracker = dh_find_tracker_by_proto(info->proto);
		if (!tracker) {	/* We need to create a new tracker */
			tracker = dh_create_tracker(info);
			if (!tracker)
				goto pass_it;
			info->tracker = (void *) tracker;
		}
	}

	switch (tracker->stat) {
	case DH_STAT_RUNNING:
		DEBUGP("xt_DYNAHELPER: helper %s is ready, let packet go\n",
		       tracker->proto);
		goto pass_it;
	case DH_STAT_LOADED:
	case DH_STAT_LOADING:
		DEBUGP("xt_DYNAHELPER: helper %s not ready, queue packet\n",
		       tracker->proto);
		goto queue_it;
	case DH_STAT_UNLOADED:
		DEBUGP("xt_DYNAHELPER: helper %s not loaded, queue packet\n",
		       tracker->proto);
		tracker->stat = DH_STAT_LOADING;
		dh_load_helper(tracker);
		if (tracker->timeout != info->timeout)
			tracker->timeout = info->timeout;
		/* Wait at most 1 second for loading helper */
		dh_start_timer(tracker, HZ);
		goto queue_it;
	}

pass_it:
	write_unlock_bh(&dh_lock);
	write_unlock_bh(&queue_lock);
	return XT_CONTINUE;

queue_it:
	write_unlock_bh(&dh_lock);
	write_unlock_bh(&queue_lock);
	(*pskb)->mark = (unsigned long) tracker;
	(*pskb)->ipvs_property = 1;
	return NF_QUEUE;
}

/****************************************************************************/
static int dh_checkentry(const char *tablename, const void *e,
	       		 const struct xt_target *target, void *targinfo,
	       		 unsigned int hook_mask)
{
	DEBUGP("xt_DYNAHELPER: checkentry\n");

	return 1;
}

/****************************************************************************/
static void dh_track(struct module *m)
{
	char *proto;
	struct dh_tracker *tracker;

	if (!m)
		return;

	if (strncmp(m->name, "nf_conntrack_", 13))
		return;
	proto = &m->name[13];

	write_lock_bh(&dh_lock);

	tracker = dh_find_tracker_by_proto(proto);
	if (tracker &&
	    tracker->stat == DH_STAT_LOADING && tracker->module != m) {
		DEBUGP("dh_track: helper %s registered\n", proto);
		tracker->module = m;
		tracker->stat = DH_STAT_LOADED;
		dh_start_timer(tracker, 1); /* release packets next interrupt */
	}

	write_unlock_bh(&dh_lock);
}

/****************************************************************************/
static void dh_untrack(struct module *m)
{
	struct dh_tracker *tracker;

	if (!m)
		return;

	write_lock_bh(&dh_lock);

	tracker = dh_find_tracker_by_module(m);
	if (tracker) {
		DEBUGP("dh_untrack: helper %s unregistered\n", tracker->proto);
		tracker->refcount = 0;
		tracker->module = NULL;
		tracker->stat = DH_STAT_UNLOADED;
		dh_stop_timer(tracker);
	}

	write_unlock_bh(&dh_lock);
}

/****************************************************************************/
static void dh_ref(struct module *m)
{
	struct dh_tracker *tracker;

	if (!m)
		return;

	write_lock_bh(&dh_lock);

	tracker = dh_find_tracker_by_module(m);
	if (tracker) {
		DEBUGP("dh_ref: helper %s referenced\n", tracker->proto);
		tracker->refcount++;
		dh_stop_timer(tracker);
	}

	write_unlock_bh(&dh_lock);
}

/****************************************************************************/
static void dh_unref(struct module *m)
{
	struct dh_tracker *tracker;

	if (!m)
		return;

	write_lock_bh(&dh_lock);

	tracker = dh_find_tracker_by_module(m);
	if (tracker) {
		DEBUGP("dh_unref: helper %s unreferenced\n", tracker->proto);
		if (tracker->refcount) {
			tracker->refcount--;
			if (!tracker->refcount)
				dh_start_timer(tracker, 0);
		}
	}

	write_unlock_bh(&dh_lock);
}

/****************************************************************************/
static inline int mark_cmp(struct ipq_queue_entry *e, unsigned long mark)
{
	return e->skb->mark == mark;
}

/****************************************************************************/
static void dh_release_packets(unsigned long mark)
{
	struct ipq_queue_entry *entry;

	while((entry = __ipq_find_dequeue_entry(mark_cmp, mark))) {
		entry->skb->mark = 0;
		entry->skb->ipvs_property = 0;
		ipq_issue_verdict(entry, NF_ACCEPT);
	}
}

/****************************************************************************/
static void dh_receive_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;

	DEBUGP("dh_receive_msg: received message\n");
	if (skb->len < NLMSG_SPACE(sizeof(struct ipq_peer_msg)))
		return;

	nlh = (struct nlmsghdr *) skb->data;
	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct ipq_peer_msg)))
		return;

	write_lock_bh(&dh_lock);

	if (nlh->nlmsg_pid && nlh->nlmsg_pid != dh_pid) {
		dh_pid = nlh->nlmsg_pid;
		DEBUGP("dh_receive_msg: dynahelper %d connected\n", dh_pid);
	}

	write_unlock_bh(&dh_lock);
}

/****************************************************************************/
static void dh_receive_handler(struct sock *sk, int len)
{
	do {
		struct sk_buff *skb;

		if (down_trylock(&dhnl_sem))
			return;

		while ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL) {
			dh_receive_msg(skb);
			kfree_skb(skb);
		}

		up(&dhnl_sem);

	} while (dhnl && dhnl->sk_receive_queue.qlen);
}

/****************************************************************************/
static int dh_event_handler(struct notifier_block *this, unsigned long event,
			    void *ptr)
{
	struct netlink_notify *n = ptr;

	if (event == NETLINK_URELEASE && n->protocol == NETLINK_DYNAHELPER &&
	    n->pid) {
		write_lock_bh(&dh_lock);
		if (n->pid == dh_pid) {
			DEBUGP("dh_event_handler: dynahelper terminated\n");
			dh_pid = 0;
		}
		write_unlock_bh(&dh_lock);
	}
	return NOTIFY_DONE;
}

/****************************************************************************/
static struct xt_target xt_dynahelper_reg = {
	.name = "DYNAHELPER",
	.family = AF_INET,
	.target = dh_target,
	.targetsize = sizeof(struct xt_dynahelper_info),
	.table = "raw",
	.hooks = (1 << NF_IP_PRE_ROUTING) | (1 << NF_IP_LOCAL_OUT),
	.checkentry = dh_checkentry,
	.me = THIS_MODULE,
};
#endif /* CONFIG_MIPS_BRCM */

static struct notifier_block ipq_dev_notifier = {
	.notifier_call	= ipq_rcv_dev_event,
};

static int
ipq_rcv_nl_event(struct notifier_block *this,
		 unsigned long event, void *ptr)
{
	struct netlink_notify *n = ptr;

#if defined(CONFIG_MIPS_BRCM)
	dh_event_handler(this, event, ptr);
#endif /* CONFIG_MIPS_BRCM */

	if (event == NETLINK_URELEASE &&
	    n->protocol == NETLINK_FIREWALL && n->pid) {
		write_lock_bh(&queue_lock);
		if (n->pid == peer_pid)
			__ipq_reset();
		write_unlock_bh(&queue_lock);
	}
	return NOTIFY_DONE;
}

static struct notifier_block ipq_nl_notifier = {
	.notifier_call	= ipq_rcv_nl_event,
};

static struct ctl_table_header *ipq_sysctl_header;

static ctl_table ipq_table[] = {
	{
		.ctl_name	= NET_IPQ_QMAX,
		.procname	= NET_IPQ_QMAX_NAME,
		.data		= &queue_maxlen,
		.maxlen		= sizeof(queue_maxlen),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{ .ctl_name = 0 }
};

static ctl_table ipq_dir_table[] = {
	{
		.ctl_name	= NET_IPV4,
		.procname	= "ipv4",
		.mode		= 0555,
		.child		= ipq_table
	},
	{ .ctl_name = 0 }
};

static ctl_table ipq_root_table[] = {
	{
		.ctl_name	= CTL_NET,
		.procname	= "net",
		.mode		= 0555,
		.child		= ipq_dir_table
	},
	{ .ctl_name = 0 }
};

#ifdef CONFIG_PROC_FS
static int
ipq_get_info(char *buffer, char **start, off_t offset, int length)
{
	int len;

	read_lock_bh(&queue_lock);

	len = sprintf(buffer,
		      "Peer PID          : %d\n"
		      "Copy mode         : %hu\n"
		      "Copy range        : %u\n"
		      "Queue length      : %u\n"
		      "Queue max. length : %u\n"
		      "Queue dropped     : %u\n"
		      "Netlink dropped   : %u\n",
		      peer_pid,
		      copy_mode,
		      copy_range,
		      queue_total,
		      queue_maxlen,
		      queue_dropped,
		      queue_user_dropped);

	read_unlock_bh(&queue_lock);

	*start = buffer + offset;
	len -= offset;
	if (len > length)
		len = length;
	else if (len < 0)
		len = 0;
	return len;
}
#endif /* CONFIG_PROC_FS */

static struct nf_queue_handler nfqh = {
	.name	= "ip_queue",
	.outfn	= &ipq_enqueue_packet,
};

static int __init ip_queue_init(void)
{
	int status = -ENOMEM;
	struct proc_dir_entry *proc;

	netlink_register_notifier(&ipq_nl_notifier);
	ipqnl = netlink_kernel_create(NETLINK_FIREWALL, 0, ipq_rcv_sk,
				      THIS_MODULE);
	if (ipqnl == NULL) {
		printk(KERN_ERR "ip_queue: failed to create netlink socket\n");
		goto cleanup_netlink_notifier;
	}

	proc = proc_net_create(IPQ_PROC_FS_NAME, 0, ipq_get_info);
	if (proc)
		proc->owner = THIS_MODULE;
	else {
		printk(KERN_ERR "ip_queue: failed to create proc entry\n");
		goto cleanup_ipqnl;
	}

	register_netdevice_notifier(&ipq_dev_notifier);
	ipq_sysctl_header = register_sysctl_table(ipq_root_table);

	status = nf_register_queue_handler(PF_INET, &nfqh);
	if (status < 0) {
		printk(KERN_ERR "ip_queue: failed to register queue handler\n");
		goto cleanup_sysctl;
	}

#if defined(CONFIG_MIPS_BRCM)
	dhnl = netlink_kernel_create(NETLINK_DYNAHELPER, 0,
				     dh_receive_handler, THIS_MODULE);
	if (dhnl == NULL) {
		printk(KERN_ERR "ip_queue_init: failed to create dynahelper "
		       "netlink socket\n");
		goto cleanup_sysctl;
	}

	status = xt_register_target(&xt_dynahelper_reg);
	if (status < 0) {
		printk(KERN_ERR "ip_queue_init: failed to register dynahelper "
		       "target\n");
		goto cleanup_dh_netlink;
	}

        /* Set hooks */
	dynahelper_track = dh_track; 
	dynahelper_untrack = dh_untrack;
	dynahelper_ref = dh_ref;
	dynahelper_unref = dh_unref;
#endif /* CONFIG_MIPS_BRCM */

	return status;

#if defined(CONFIG_MIPS_BRCM)
cleanup_dh_netlink:
	sock_release(dhnl->sk_socket);
	down(&dhnl_sem);
	up(&dhnl_sem);
	dh_destroy_trackers();
#endif /* CONFIG_MIPS_BRCM */

cleanup_sysctl:
	unregister_sysctl_table(ipq_sysctl_header);
	unregister_netdevice_notifier(&ipq_dev_notifier);
	proc_net_remove(IPQ_PROC_FS_NAME);

cleanup_ipqnl:
	sock_release(ipqnl->sk_socket);
	mutex_lock(&ipqnl_mutex);
	mutex_unlock(&ipqnl_mutex);

cleanup_netlink_notifier:
	netlink_unregister_notifier(&ipq_nl_notifier);
	return status;
}

static void __exit ip_queue_fini(void)
{
#if defined(CONFIG_MIPS_BRCM)
	dynahelper_track = NULL;
	dynahelper_untrack = NULL;
	dynahelper_ref = NULL;
	dynahelper_unref = NULL;
	xt_unregister_target(&xt_dynahelper_reg);

	sock_release(dhnl->sk_socket);
	down(&dhnl_sem);
	up(&dhnl_sem);
	dh_destroy_trackers();
#endif /* CONFIG_MIPS_BRCM */

	nf_unregister_queue_handlers(&nfqh);
	synchronize_net();
	ipq_flush(NF_DROP);

	unregister_sysctl_table(ipq_sysctl_header);
	unregister_netdevice_notifier(&ipq_dev_notifier);
	proc_net_remove(IPQ_PROC_FS_NAME);

	sock_release(ipqnl->sk_socket);
	mutex_lock(&ipqnl_mutex);
	mutex_unlock(&ipqnl_mutex);

	netlink_unregister_notifier(&ipq_nl_notifier);
}

MODULE_DESCRIPTION("IPv4 packet queue handler");
MODULE_AUTHOR("James Morris <jmorris@intercode.com.au>");
MODULE_LICENSE("GPL");

module_init(ip_queue_init);
module_exit(ip_queue_fini);
