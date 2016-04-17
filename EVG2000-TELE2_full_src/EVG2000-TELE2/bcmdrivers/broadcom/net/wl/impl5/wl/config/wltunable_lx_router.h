/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wltunable_lx_router.h,v 1.3.98.1 2008/11/06 21:32:02 Exp $
 *
 * wl driver tunables
 */

#define NRXBUFPOST	56	/* # rx buffers posted */

#define RXBND	24	/* max # rx frames to process */

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1

#if defined(DSLCPE) && defined(DSLCPE_SDIO)
#undef NRXBUFPOST
#define NRXBUFPOST	32
#undef RXBND
#define RXBND		12
#define TXSBND		16
#endif

