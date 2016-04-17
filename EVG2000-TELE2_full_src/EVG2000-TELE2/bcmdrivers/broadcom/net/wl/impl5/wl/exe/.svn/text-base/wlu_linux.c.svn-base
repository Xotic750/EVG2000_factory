/*
 * Linux port of wl command line utility
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlu_linux.c,v 1.28 2007/09/27 19:42:41 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlu.h"

#define DEV_TYPE_LEN 3 /* length for devtype 'wl'/'et' */

static cmd_t *wl_find_cmd(char* name);
static int do_interactive(struct ifreq *ifr);
static int wl_do_cmd(struct ifreq *ifr, char **argv);

static void
syserr(char *s)
{
	fprintf(stderr, "%s: ", av0);
	perror(s);
	
#ifdef DSLCPE_SHLIB
	return;
#else	
	exit(errno);
#endif	
}

static int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct ifreq *ifr = (struct ifreq *) wl;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ifr->ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
		if (cmd != WLC_GET_MAGIC) {
			ret = IOCTL_ERROR;
		}
	}

	/* cleanup */
	close(s);
	return ret;
}

static int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* get device type */
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			syserr("wl_get_dev_type");

		*(char *)buf = '\0';
	}
	else
		strncpy(buf, info.driver, len);

	close(s);
	return ret;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
	return wl_ioctl(wl, cmd, buf, len, FALSE);
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	return wl_ioctl(wl, cmd, buf, len, TRUE);
}

static void
wl_find(struct ifreq *ifr)
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[1000], *c, *name;
	char dev_type[DEV_TYPE_LEN];

	ifr->ifr_name[0] = '\0';

	if (!(fp = fopen(proc_net_dev, "r")))
		return;

	/* eat first two lines */
	if (!fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		strncpy(ifr->ifr_name, name, IFNAMSIZ);
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			!strncmp(dev_type, "wl", 2))
			if (wl_check((void *) ifr) == 0)
				break;
		ifr->ifr_name[0] = '\0';
	}

	fclose(fp);
}

/* this is a bit of a hack, assume args never goes beyond 15 */
#define INTERACTIVE_NUM_ARGS		15
#define INTERACTIVE_MAX_INPUT_LENGTH	256


#if defined (DSLCPE)
int
wl_libmain(int argc, char **argv)
#else
int
main(int argc, char **argv)
#endif
{
	struct ifreq ifr;
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname = NULL;
	int help = 0;
	int status = CMD_WL;

	av0 = argv[0];

	memset(&ifr, 0, sizeof(ifr));

	for (++argv; *argv;) {

		/* command option */
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
			if (help)
				break;
			if (ifname)
				strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
			continue;
		}
		/* parse error */
		else if (status == CMD_ERR)
			break;
		/* wl command */
		/*
		 * else if (status == CMD_WL)
		 *	;
		 */

		/* use default interface */
		if (!*ifr.ifr_name)
			wl_find(&ifr);
		/* validate the interface */
		if (!*ifr.ifr_name || wl_check((void *)&ifr)) {
			fprintf(stderr, "%s: wl driver adapter not found\n", av0);
#ifdef DSLCPE_SHLIB
			return -1;
#else			
			exit(1);
#endif			
		}

		if (strcmp (*argv, "--interactive") == 0) {
			err = do_interactive(&ifr);
			return err;
		}


		/* search for command */
		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd)
			cmd = &wl_varcmd;

		/* do command */
		err = (*cmd->func)((void *) &ifr, cmd, argv);

		break;
	}

	if (help && *argv) {
		cmd = wl_find_cmd(*argv);
		if (cmd)
			cmd_usage(stdout, cmd);
		else {
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       av0, *argv);
		}
	} else if (!cmd)
		wl_usage(stdout, NULL);
	else if (err == USAGE_ERROR)
		cmd_usage(stderr, cmd);
	else if (err == IOCTL_ERROR)
		wl_printlasterror((void *) &ifr);
	return err;
}

static int
do_interactive(struct ifreq *ifr)
{
	int err = 0;

	while (1) {
		char *fgsret;
		char line[INTERACTIVE_MAX_INPUT_LENGTH];

		/* read a line from stdin, try and process it */
		printf("> ");
		fgsret = fgets(line, sizeof(line), stdin);

		/* end of file */
		if ((fgsret == NULL) || (line[0] == '\n')) {
			break;
		}

		if (strlen (line) > 0) {
			/* skip past first arg if it's "wl" and parse up arguments */
			char *argv[INTERACTIVE_NUM_ARGS];
			int argc;
			char *token;

			argc = 0;
			while ((argc < (INTERACTIVE_NUM_ARGS - 1)) &&
			       ((token = strtok(argc ? NULL : line, " \t\n")) != NULL)) {
				argv[argc++] = token;
			}
			argv[argc] = NULL;

			if (strcmp(argv[0], "q") == 0 ||
			    strcmp(argv[0], "exit") == 0) {
				break;
			}

			err = wl_do_cmd(ifr, argv);
		}
	}
	return err;
}


/*
 * find command in argv and execute it
 * Won't handle changing ifname yet, expects that to happen with the --interactive
 * Return an error if unable to find/execute command
 */
static int
wl_do_cmd(struct ifreq *ifr, char **argv)
{
	cmd_t *cmd = NULL;
	int err = 0;
	int help = 0;
	char *ifname = NULL;
	int status = CMD_WL;

	/* skip over 'wl' if it's there */
	if (*argv && strcmp (*argv, "wl") == 0) {
		argv++;
	}

	/* handle help or interface name changes */
	if (*argv && (status = wl_option (&argv, &ifname, &help)) == CMD_OPT) {
		if (ifname) {
			fprintf(stderr,
			        "Interface name change not allowed within --interactive\n");
		}
	}

	/* in case wl_option eats all the args */
	if (!*argv) {
		return err;
	}

	if (status != CMD_ERR) {
		/* search for command */
		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd) {
			cmd = &wl_varcmd;
		}

		/* do command */
		err = (*cmd->func)((void *)ifr, cmd, argv);
	}

	return err;
}

/* Search the wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t *
wl_find_cmd(char* name)
{
	cmd_t *cmd = NULL;

	/* search the wl_cmds for a matching name */
	for (cmd = wl_cmds; cmd->name && strcmp(cmd->name, name); cmd++);

	if (cmd->name == NULL)
		cmd = NULL;

	return cmd;
}
