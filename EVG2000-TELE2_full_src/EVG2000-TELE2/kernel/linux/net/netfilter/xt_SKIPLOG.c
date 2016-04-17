/* This is a module which is used for stopping logging. */

/* 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#if defined(CONFIG_MIPS_BRCM)
#include <linux/blog.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("iptables stop logging module");
MODULE_ALIAS("ipt_SKIPLOG");

static unsigned int
skiplog_tg(struct sk_buff **pskb,
	  const struct net_device *in,
	  const struct net_device *out,
	  unsigned int hooknum,
	  const struct xt_target *target,
	  const void *targinfo)
{
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	blog_skip(*pskb);
#endif

	return XT_CONTINUE;
}

static struct xt_target xt_skiplog_target[] = {
	{
		.name		= "SKIPLOG",
		.family		= AF_INET,
		.revision	= 0,
		.target		= skiplog_tg,
		.me		= THIS_MODULE,
	},
};

static int __init xt_skiplog_init(void)
{
	return xt_register_targets(xt_skiplog_target, ARRAY_SIZE(xt_skiplog_target));
}

static void __exit xt_skiplog_fini(void)
{
	xt_unregister_targets(xt_skiplog_target, ARRAY_SIZE(xt_skiplog_target));
}

module_init(xt_skiplog_init);
module_exit(xt_skiplog_fini);
