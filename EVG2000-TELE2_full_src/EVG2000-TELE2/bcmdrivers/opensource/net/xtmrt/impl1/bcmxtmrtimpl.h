/*
<:copyright-gpl 
 Copyright 2007 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/**************************************************************************
 * File Name  : bcmxtmrtimpl.h
 *
 * Description: This file contains constant definitions and structure
 *              definitions for the BCM6368 ATM/PTM network device driver.
 ***************************************************************************/

#if !defined(_BCMXTMRTIMPL_H)
#define _BCMXTMRTIMPL_H


#define CHIP_REV_BCM6368B0          0x636800b0
#define MAX_DEV_CTXS                16
#define MAX_MATCH_IDS               128
#define MAX_DEFAULT_MATCH_IDS       16
#define MAX_MTU_SIZE                (1500 + 14 + 4)
#define MAX_RFC2684_HDR_SIZE        10
#define SAR_DMA_MAX_BURST_LENGTH    8
#define RXBUF_HEAD_RESERVE          32
#define RXBUF_SIZE                  SKB_DATA_ALIGN(MAX_MTU_SIZE +       \
                                        MAX_RFC2684_HDR_SIZE +          \
                                        SAR_DMA_MAX_BURST_LENGTH)
#define RXBUF_ALLOC_SIZE            SKB_DATA_ALIGN(RXBUF_HEAD_RESERVE + \
                                        RXBUF_SIZE+                     \
                                        sizeof(struct skb_shared_info))
#define MAX_BUFMEM_BLOCKS           64
#define SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define RFC1626_MTU                 9180

#define SAR_RX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_0
#define SAR_TX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_4
#define SAR_RX_DMA_BASE_CHAN        0
#define NR_SAR_RX_DMA_CHANS         2
#define SAR_TX_DMA_BASE_CHAN        4
#define NR_SAR_TX_DMA_CHANS         16
#define SAR_TIMEOUT                 (HZ/20)
#define INVALID_VCID                0xff

#define CACHE_TO_NONCACHE(x)        KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)        KSEG0ADDR(x)

#define XTMRT_UNINITIALIZED         0
#define XTMRT_INITIALIZED           1
#define XTMRT_RUNNING               2

#define SKB_PROTO_ATM_CELL          0xf000
#define XTM_POLL_DONE               0x80000000

/* Circuit types. */
#define XCT_TRANSPARENT             0x00000001
#define XCT_AAL0_PKT                0x00000002
#define XCT_AAL0_CELL               0x00000003
#define XCT_OAM_F5_SEG              0x00000004
#define XCT_OAM_F5_E2E              0x00000005
#define XCT_RM                      0x00000006
#define XCT_AAL5                    0x00000007
#define XCT_ASM_P0                  0x00000008
#define XCT_ASM_P1                  0x00000009
#define XCT_ASM_P2                  0x0000000a
#define XCT_ASM_P3                  0x0000000b
#define XCT_OAM_F4_SEG              0x0000000c
#define XCT_OAM_F4_E2E              0x0000000d
#define XCT_TEQ                     0x0000000e
#define XCT_PTM                     0x0000000f

/* Transmit Buffer Descriptor frame status word for ATM/PTM. */
#define FSTAT_MASK                  0x00000fff
#define FSTAT_ATM_VCID_MASK         0x0000000f
#define FSTAT_ATM_VCID_SHIFT        0
#define FSTAT_PTM_CRC               0x00000001
#define FSTAT_PTM_ENET_FCS          0x00000002
#define FSTAT_CT_MASK               0x000000f0
#define FSTAT_CT_SHIFT              4
#define FSTAT_CT_TRANSPARENT        (XCT_TRANSPARENT << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL0_PKT           (XCT_AAL0_PKT    << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL0_CELL          (XCT_AAL0_CELL   << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F5_SEG         (XCT_OAM_F5_SEG  << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F5_E2E         (XCT_OAM_F5_E2E  << FSTAT_CT_SHIFT)
#define FSTAT_CT_RM                 (XCT_RM          << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL5               (XCT_AAL5        << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P0             (XCT_ASM_P0      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P1             (XCT_ASM_P1      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P2             (XCT_ASM_P2      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P3             (XCT_ASM_P3      << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F4_SEG         (XCT_OAM_F4_SEG  << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F4_E2E         (XCT_OAM_F4_E2E  << FSTAT_CT_SHIFT)
#define FSTAT_CT_TEQ                (XCT_TEQ         << FSTAT_CT_SHIFT)
#define FSTAT_CT_PTM                (XCT_PTM         << FSTAT_CT_SHIFT)
#define FSTAT_COMMON_INS_HDR_EN     0x00000100
#define FSTAT_COMMON_HDR_INDEX_MASK 0x00000600
#define FSTAT_COMMON_HDR_INDEX_SHIFT 9
#define FSTAT_INDEX_CI              0x00000100
#define FSTAT_INDEX_CLP             0x00000200
#define FSTAT_INDEX_USE_ALT_GFC     0x00000400
#define FSTAT_MODE_INDEX            0x00000000
#define FSTAT_MODE_COMMON           0x00000800

/* Receive Buffer Descriptor frame status word for ATM/PTM. */
#define FSTAT_MATCH_ID_MASK         0x0000007f             
#define FSTAT_MATCH_ID_SHIFT        0
#define FSTAT_PACKET_CELL_MASK      0x00000400
#define FSTAT_PACKET                0x00000000
#define FSTAT_CELL                  0x00000400
#define FSTAT_ERROR                 0x00000800

/* First byte of a received cell. */
#define CHDR_CT_MASK                0x0f
#define CHDR_CT_SHIFT               0
#define CHDR_CT_OAM_F5_SEG          (XCT_OAM_F5_SEG  << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F5_E2E          (XCT_OAM_F5_E2E  << CHDR_CT_SHIFT)
#define CHDR_CT_RM                  (XCT_RM          << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P0              (XCT_ASM_P0      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P1              (XCT_ASM_P1      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P2              (XCT_ASM_P2      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P3              (XCT_ASM_P3      << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F4_SEG          (XCT_OAM_F4_SEG  << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F4_E2E          (XCT_OAM_F4_E2E  << CHDR_CT_SHIFT)
#define CHDR_PORT_MASK              0x60
#define CHDR_PORT_SHIFT             5
#define CHDR_ERROR                  0x80
#define CHDR_ERROR_MISC             0x01
#define CHDR_ERROR_CRC              0x02
#define CHDR_ERROR_CAM              0x04
#define CHDR_ERROR_HEC              0x08
#define CHDR_ERROR_PORT             0x10

static UINT32 gs_ulLogPorts[]  = {0, 3, 1, 2};
#define PORT_PHYS_TO_LOG(PP) gs_ulLogPorts[PP]

/* Information about a DMA transmit channel. A device instance may use more
 * than one transmit DMA channel. A DMA channel corresponds to a transmit queue.
 */
typedef struct TxQInfo
{
    UINT32 ulPort;
    UINT32 ulPtmPriority;
    UINT32 ulSubPriority;
    UINT32 ulQueueSize;
    UINT32 ulDmaIndex;

    UINT8 *pMemBuf;
    volatile DmaChannelCfg *pDma;
    volatile DmaDesc *pBds;
    struct sk_buff **ppSkbs;
    UINT32 ulNumTxBufsQdOne;
    UINT32 ulFreeBds;
    UINT32 ulHead;
    UINT32 ulTail;
} TXQINFO, *PTXQINFO;

/* The definition of the driver control structure */
typedef struct bcmxtmrt_dev_context
{
    /* Linux structures. */
    struct net_device *pDev;        
    struct net_device_stats DevStats;
    IOCTL_MIB_INFO MibInfo;
    struct ppp_channel Chan;

    /* ATM/PTM fields. */
    XTM_ADDR Addr;
    UINT32 ulLinkState;
    UINT32 ulOpenState;
    UINT32 ulAdminStatus;
    UINT32 ulHdrType;
    UINT32 ulEncapType; /* IPoA, PPPoA, or EoA[bridge,MER,PPPoE] */
    UINT32 ulFlags;

    /* Transmit fields. */
    UINT8 ucTxVcid;
    UINT32 ulTxQInfosSize;
    TXQINFO TxQInfos[MAX_TRANSMIT_QUEUES];
    PTXQINFO pTxPriorities[MAX_PTM_PRIORITIES][MAX_PHY_PORTS][MAX_SUB_PRIORITIES];

    /*Port Mirroring fields*/
    char szMirrorIntfIn[MIRROR_INTF_SIZE];
    char szMirrorIntfOut[MIRROR_INTF_SIZE];

#ifdef ATM_BONDING_TEST
	 UINT32 portAsm ;
	 UINT32 sendAsm ;
	 UINT32 asmFlag ;
#endif
} BCMXTMRT_DEV_CONTEXT, *PBCMXTMRT_DEV_CONTEXT;

typedef struct RxBdInfo
{
    volatile DmaDesc *pBdBase;
    volatile DmaDesc *pBdHead;
    volatile DmaDesc *pBdTail;
    volatile DmaChannelCfg *pDma;
} RXBDINFO, *PRXBDINFO;

#ifdef PTM_BONDING_TEST
typedef struct _PtmHeader {
   UINT16 LineSel    : 1 ;
   UINT16 Reserved   : 2 ;
   UINT16 PktEop     : 1 ;
   UINT16 FragSize   : 12 ;
} PTMBondHeader ;
#endif

/* Information that is global to all network device instances. */
typedef struct bcmxtmrt_global_info
{
    /* Linux structures. */
    PBCMXTMRT_DEV_CONTEXT pDevCtxs[MAX_DEV_CTXS];
    PBCMXTMRT_DEV_CONTEXT pDevCtxsByMatchId[MAX_MATCH_IDS];
    struct timer_list Timer;
    struct atm_dev *pAtmDev;

    /* DMA, BD and buffer fields. */
    volatile DmaRegs *pDmaRegs;
    volatile DmaChannelCfg *pTxDmaBase;
    volatile DmaChannelCfg *pRxDmaBase;
    volatile DmaDesc *pRxBdMem;
    RXBDINFO RxBdInfos[MAX_RECEIVE_QUEUES];
    struct sk_buff *pFreeRxSkbList;
    UINT8 *pBufMem[MAX_BUFMEM_BLOCKS];
    UINT32 ulIntEnableMask;

    /* Global transmit queue fields. */
    UINT32 ulNumExtBufs;
    UINT32 ulNumExtBufsRsrvd;
    UINT32 ulNumExtBufs90Pct;
    UINT32 ulNumExtBufs50Pct;
    UINT32 ulNumTxQs;
    UINT32 ulNumTxBufsQdAll;
    UINT32 ulDbgQ1;
    UINT32 ulDbgQ2;
    UINT32 ulDbgQ3;
    UINT32 ulDbgD1;
    UINT32 ulDbgD2;
    UINT32 ulDbgD3;

    /* Callback functions. */
    XTMRT_CALLBACK pfnOamHandler;
    void *pOamContext;
    XTMRT_CALLBACK pfnAsmHandler;
    void *pAsmContext;

    /* MIB counter registers. */
    UINT32 *pulMibTxOctetCountBase;
    UINT32 *pulMibRxMatch;
    UINT32 *pulMibRxOctetCount;
    UINT32 *pulMibRxPacketCount;

    /* Everything else. */
    UINT32 ulChipRev;
    UINT32 ulDrvState;
#ifdef PTM_BONDING_TEST
    PTMBondHeader ptmBondHdr [8] ;
    int fragmentSz ;
#endif
    int rcvPktCount ;
    UINT8 PhyDump [2] [11008] ;
} BCMXTMRT_GLOBAL_INFO, *PBCMXTMRT_GLOBAL_INFO;

#if defined(CONFIG_BCM96368)
#if defined(PTM_BONDING_TEST) || defined(ATM_BONDING_TEST)
#define ADSL_LMEM_BASE					0xB0F80000L
#define DSL_PHY_LMEM_BASE				0x19000000L
#define DSL_PHY_SDRAM_BASE				0x10000000L
#define DSL_PHY_SDRAM_ADDR_MASK			0xF0000000

#define ADSL_MIPS_LMEM_ADDR(a)			(((ulong)(a) & DSL_PHY_LMEM_BASE) == DSL_PHY_LMEM_BASE)
#define LMEM_ADDR_TO_HOST(a)                    ((void *) ((ulong) (a) + (ADSL_LMEM_BASE - ((ulong)(a) & 0xff000000))))
#define ADSL_ADDR_TO_HOST(addr)                 ADSL_MIPS_LMEM_ADDR(addr) ?  LMEM_ADDR_TO_HOST(addr) : addr
#endif
#endif

#endif /* _BCMXTMRTIMPL_H */

