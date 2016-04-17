/*
 * OS independent remote wl declarations
 *
 * Copyright 2002, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id: wlu_remote.h,v 1.5 2008/03/28 22:04:20 Exp $
 */

/* Remote wl */
#define NO_REMOTE 	0
#define REMOTE_SERIAL 	1
#define REMOTE_SOCKET 	2

/* Used in cdc_ioctl_t.flags field */
#define REMOTE_SET_IOCTL 	1
#define REMOTE_GET_IOCTL	2
#define REMOTE_REPLY		4

/* Used to send ioctls over the transport pipe */
typedef struct remote_ioctl {
	cdc_ioctl_t 	msg;
	uint		data_len;
} rem_ioctl_t;
#define REMOTE_SIZE	sizeof(rem_ioctl_t)

extern int remote_type;
void *open_pipe(int rem_type, char *port, int ReadTotalTimeout, int debug);
extern int remote_CDC_tx(void *wl, uint cmd,
                         uchar *buf, uint buf_len,
                         uint data_len, uint flags, int debug);
extern rem_ioctl_t *remote_CDC_rx_hdr(void *remote, int debug);
extern int remote_CDC_rx(void *wl, rem_ioctl_t *rem_ptr,
                         uchar *readbuf, uint buflen, int debug);
