/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *                                     
 * $Id$
 *
 * wl driver tunables
 */
#ifndef BCMDBUS
/*From wltunable_lx_router.h */

#define NRXBUFPOST	56	/* # rx buffers posted */

#define RXBND	24	/* max # rx frames to process */

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1

#if defined(DSLCPE) && defined(DSLCPE_SDIO)
/*These parameter is tuned for DSL SDIO*/
#undef NRXBUFPOST
#define NRXBUFPOST	32
#undef RXBND
#define RXBND		12
#define TXSBND		16
#endif
#else
/*From wltunable_rte_4322_bmac.h */
#define D11CONF		0x10000		/* d11 core rev 16 */
#define ACONF		0
#define GCONF		0
#define NCONF		0x10		/* N-PHY 4 only, change to 0x18 for rev3 */
#define LPCONF		0
#define SSLPNCONF       0

#define NTXD		64	/* THIS HAS TO MATCH with HIGH driver tunable */
#define NRXD		32
#define NRXBUFPOST	16
#define WLC_DATAHIWAT	10
#define RXBND		16
#define NRPCTXBUFPOST	48
#endif
