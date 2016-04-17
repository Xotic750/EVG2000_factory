/*
 * OS Abstraction Layer
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: osl.h,v 13.38.2.3 2009/02/26 17:07:08 Exp $
 */

#ifndef _osl_h_
#define _osl_h_

/* osl handle type forward declaration */
typedef struct osl_info osl_t;
typedef struct osl_dmainfo osldma_t;

#define OSL_PKTTAG_SZ	32 /* Size of PktTag */

/* Drivers use PKTFREESETCB to register a callback function when a packet is freed by OSL */
typedef void (*pktfree_cb_fn_t)(void *ctx, void *pkt, unsigned int status);

#ifdef OSLREGOPS
/* Drivers use REGOPSSET() to register register read/write funcitons */
typedef unsigned int (*osl_rreg_fn_t)(void *ctx, void *reg, unsigned int size);
typedef void  (*osl_wreg_fn_t)(void *ctx, void *reg, unsigned int val, unsigned int size);
#endif

#if defined(__ECOS)
#include <ecos_osl.h>
#elif  defined(DOS)
#include <dos_osl.h>
#elif defined(PCBIOS)
#include <pcbios_osl.h>
#elif defined(linux)
#ifdef USER_MODE
#include <usermode_osl.h>
#else
#include <linux_osl.h>
#endif
#elif defined(NDIS)
#include <ndis_osl.h>
#elif defined(_CFE_)
#include <cfe_osl.h>
#elif defined(_MINOSL_)
#include <min_osl.h>
#elif defined(MACOSX)
#include <macosx_osl.h>
#elif defined(__NetBSD__)
#include <bsd_osl.h>
#elif defined(EFI)
#include <efi_osl.h>
#else
#error "Unsupported OSL requested"
#endif 

/* handy */
#define	SET_REG(osh, r, mask, val)	W_REG((osh), (r), ((R_REG((osh), r) & ~(mask)) | (val)))

#ifdef DSLCPE_SDIO_EBIDMA
#ifdef HOST_W_REG
#define HOST_SET_REG(r, mask, val) HOST_W_REG((r), ((HOST_R_REG(r) & ~(mask)) | (val)))
#endif
#endif

#if !defined(OSL_SYSUPTIME)
#define OSL_SYSUPTIME() (0)
#define OSL_SYSUPTIME_SUPPORT FALSE
#else
#define OSL_SYSUPTIME_SUPPORT TRUE
#endif /* OSL_SYSUPTIME */

#endif	/* _osl_h_ */
