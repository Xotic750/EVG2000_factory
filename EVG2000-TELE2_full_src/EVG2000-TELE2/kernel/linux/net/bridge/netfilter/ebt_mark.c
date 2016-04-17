/*
 *  ebt_mark
 *
 *	Authors:
 *	Bart De Schuymer <bdschuym@pandora.be>
 *
 *  July, 2002
 *
 */

/* The mark target can be used in any chain,
 * I believe adding a mangle table just for marking is total overkill.
 * Marking a frame doesn't really change anything in the frame anyway.
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>
#include <linux/module.h>

static int ebt_target_mark(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_mark_t_info *info = (struct ebt_mark_t_info *)data;
	int action = info->target & -16;

	if (action == MARK_SET_VALUE)
		(*pskb)->mark = info->mark;
	else if (action == MARK_OR_VALUE)
		(*pskb)->mark |= info->mark;
	else if (action == MARK_AND_VALUE)
		(*pskb)->mark &= info->mark;
	else if (action == MARK_XOR_VALUE)
		(*pskb)->mark ^= info->mark;
	else  //brcm
		(*pskb)->vtag = (unsigned short)(info->mark);

// brcm -- begin
   /* if this is an 8021Q frame and skb->vtag is not zero, we need to do 8021p priority
    * or vlan id remarking.
    */
	if (((*pskb)->protocol == __constant_htons(ETH_P_8021Q)) && (*pskb)->vtag) {
      /* we want to save the original vlan tag before remarking it with the QoS matching
       * rule, so that it can be restored in case this frame egresses to a vlan interface
       * and we need to do double tagging.
       */
      /* bit 12 of skb->vtag_save tells whether this frame had been remarked or not. */
      if (((*pskb)->vtag_save & 0x1000) == 0) {

   	   struct vlan_hdr *frame;
	      unsigned short TCI;

		   frame = (struct vlan_hdr *)((*pskb)->nh.raw);
		   TCI = ntohs(frame->h_vlan_TCI);

         /* set bit 12 of vtag_save to indicate that this vlan frame has been remarked. */
         (*pskb)->vtag_save = (TCI | 0x1000);

         /* if the 8021p priority field (bits 0-3) of skb->vtag is not zero, remark
          * 8021p priority of the frame.  Since the 8021p priority value in vtag had
          * been incremented by 1, we need to minus 1 from it to get the exact value.
          */
         if ((*pskb)->vtag & 0xf)
		      TCI = (TCI & 0x1fff) | ((((*pskb)->vtag & 0xf) - 1) << 13);

         /* if the vlan id field (bits 4-15) of skb->vtag is not zero, remark vlan id of
          * the frame.  Since the vlan id value in vtag had been incremented by 1, we need
          * to minus 1 from it to get the exact value.
          */
         if ((*pskb)->vtag & 0xfff0)
            TCI = (TCI & 0xf000) | ((((*pskb)->vtag >> 4) & 0xfff) - 1);

		   frame->h_vlan_TCI = htons(TCI);
      }
	}
// brcm -- end

	return info->target | ~EBT_VERDICT_BITS;
}

static int ebt_target_mark_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_mark_t_info *info = (struct ebt_mark_t_info *)data;
	int tmp;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_mark_t_info)))
		return -EINVAL;
	tmp = info->target | ~EBT_VERDICT_BITS;
	if (BASE_CHAIN && tmp == EBT_RETURN)
		return -EINVAL;
	CLEAR_BASE_CHAIN_BIT;
	if (tmp < -NUM_STANDARD_TARGETS || tmp >= 0)
		return -EINVAL;
	tmp = info->target & ~EBT_VERDICT_BITS;
	if (tmp != MARK_SET_VALUE && tmp != MARK_OR_VALUE &&
	    tmp != MARK_AND_VALUE && tmp != MARK_XOR_VALUE &&
            tmp != VTAG_SET_VALUE)
		return -EINVAL;
	return 0;
}

static struct ebt_target mark_target =
{
	.name		= EBT_MARK_TARGET,
	.target		= ebt_target_mark,
	.check		= ebt_target_mark_check,
	.me		= THIS_MODULE,
};

static int __init ebt_mark_init(void)
{
	return ebt_register_target(&mark_target);
}

static void __exit ebt_mark_fini(void)
{
	ebt_unregister_target(&mark_target);
}

module_init(ebt_mark_init);
module_exit(ebt_mark_fini);
MODULE_LICENSE("GPL");
