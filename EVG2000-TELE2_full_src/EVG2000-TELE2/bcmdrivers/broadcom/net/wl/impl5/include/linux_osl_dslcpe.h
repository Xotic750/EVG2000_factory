/*
 * Linux OS Independent Layer
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: linux_osl_dslcpe.h,v Exp $
 */

#ifndef _linux_osl_dslcpe_h_
#define _linux_osl_dslcpe_h_

#if defined(DSLCPE_DELAY)
    #include <linux/spinlock_types.h>

    typedef struct {
        int long_delay;
        spinlock_t *lock;
    #ifdef DSLCPE_SDIO	
        ulong *flags;
    #endif	
        void *wl;
        unsigned long MIPS;	
    } shared_osl_t;

    extern void osl_oshsh_init(osl_t *osh, shared_osl_t *oshsh);
    extern void osl_long_delay(osl_t *osh, uint usec, bool yield, int tag);
    extern int in_long_delay(osl_t *osh);

    #define	OSL_LONG_DELAY(osh, usec)		osl_long_delay(osh, usec, 0, -1)
    #define OSL_YIELD_EXEC(osh, usec, tag)	osl_long_delay(osh, usec, 1, tag)
    #define IN_LONG_DELAY(osh)				in_long_delay(osh)
#endif

#ifdef DSLCPE_SDIO_EBIDMA
    #define SDIOH_R_REG(r) ( \
        sizeof(*(r)) == sizeof(uint8) ? readb((volatile uint8*)(r)) : \
        sizeof(*(r)) == sizeof(uint16) ? readw((volatile uint16*)(r)) : \
        readl((volatile uint32*)(r)) \
    )

    #define SDIOH_W_REG(r, v) do { \
        switch (sizeof(*(r))) { \
        case sizeof(uint8):	writeb((uint8)(v), (volatile uint8*)(r)); break; \
        case sizeof(uint16):	writew((uint16)(v), (volatile uint16*)(r)); break; \
        case sizeof(uint32):	writel((uint32)(v), (volatile uint32*)(r)); break; \
        } \
    } while (0)

    #define HOST_R_REG			SDIOH_R_REG
    #define HOST_W_REG			SDIOH_W_REG
    #define	HOST_AND_REG(r, v)	HOST_W_REG((r), HOST_R_REG(r) & (v))
    #define	HOST_OR_REG(r, v)	HOST_W_REG((r), HOST_R_REG(r) | (v))
    #undef R_SM
    #undef W_SM
    #define R_SM				HOST_R_REG
    #define W_SM				HOST_W_REG
#endif

extern int  osl_pktq_len(osl_t *osh);
extern bool osl_pktq_wmark_low_drop(osl_t *osh);
extern bool osl_pktq_wmark_high_drop(osl_t *osh);
extern void osl_pktpreallocinc(osl_t *osh, void *skb);

#define PRIO_LOC_NFMARK 16
#define PKT_PREALLOCINC(osh, skb) osl_pktpreallocinc((osh), (skb))	

#endif	/* _linux_osl_dslcpe_h_ */
