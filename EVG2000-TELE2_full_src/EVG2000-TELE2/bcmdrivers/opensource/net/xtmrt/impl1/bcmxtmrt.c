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
 * File Name  : bcmxtmrt.c
 *
 * Description: This file implements BCM6368 ATM/PTM network device driver
 *              runtime processing - sending and receiving data.
 ***************************************************************************/


/* Defines. */
#define CARDNAME    "bcmxtmrt"
#define VERSION     "0.1"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__


/* Includes. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmnet.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include "bcmxtmrtimpl.h"
#include <asm/io.h>
#include <asm/r4kcache.h>
#include <asm/uaccess.h>
#include <linux/blog.h>     /* CONFIG_BLOG */


/* Externs. */
extern unsigned long getMemorySize(void);


/* Prototypes. */
static inline void cache_wbflush_region(void *addr, void *end);
static inline void cache_wbflush_len(void *addr, int len);
int __init bcmxtmrt_init( void );
static void bcmxtmrt_cleanup( void );
static int bcmxtmrt_open( struct net_device *dev );
static int bcmxtmrt_close( struct net_device *dev );
static void bcmxtmrt_timeout( struct net_device *dev );
static struct net_device_stats *bcmxtmrt_query(struct net_device *dev);
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd);
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx,void *useraddr);
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd,
    unsigned long arg);
static PBCMXTMRT_DEV_CONTEXT FindDevCtx( short vpi, int vci );
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc);
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc);
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb);
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan,struct sk_buff *skb);
#ifdef ATM_BONDING_TEST
static void sendAsmOnPort (PBCMXTMRT_DEV_CONTEXT pDevCtx);
static void sendSkbAsAsm (PBCMXTMRT_DEV_CONTEXT pDevCtx, struct sk_buff *skb);
#endif
static int bcmxtmrt_xmit( struct sk_buff *skb, struct net_device *dev);
static void AddRfc2684Hdr(struct sk_buff **ppskb,PBCMXTMRT_DEV_CONTEXT pDevCtx);
#ifdef PTM_BONDING_TEST
static void AddPTMBondingHeader(struct sk_buff **ppskb,PBCMXTMRT_DEV_CONTEXT pDevCtx);
#endif
static void AssignRxBuffer(PRXBDINFO pRxBdInfo, UINT8 *pucData, UINT8 *pEnd);
static void bcmxtmrt_freeskbordata( PRXBDINFO pRxBdInfo, struct sk_buff *skb,
    unsigned nFlag );
static irqreturn_t bcmxtmrt_rxisr(int nIrq, void *pRxDma);
static int bcmxtmrt_poll(struct net_device * dev, int * budget);
static UINT32 bcmxtmrt_rxtask( UINT32 ulBudget, UINT32 *pulMoreToDo );
static void ProcessRxCell( PBCMXTMRT_GLOBAL_INFO pGi, PRXBDINFO pRxBdInfo,
    UINT8 *pucData );
static void MirrorPacket( struct sk_buff *skb, char *intfName );
static void bcmxtmrt_timer( PBCMXTMRT_GLOBAL_INFO pGi );
static int DoGlobInitReq( PXTMRT_GLOBAL_INIT_PARMS pGip );
static int DoCreateDeviceReq( PXTMRT_CREATE_NETWORK_DEVICE pCnd );
static int DoRegCellHdlrReq( PXTMRT_CELL_HDLR pCh );
static int DoUnregCellHdlrReq( PXTMRT_CELL_HDLR pCh );
static int DoLinkStsChangedReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoLinkUp( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoLinkDown( PBCMXTMRT_DEV_CONTEXT pDevCtx );
static int DoSetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static int DoUnsetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static int DoSendCellReq( PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC );
static int DoDeleteDeviceReq( PBCMXTMRT_DEV_CONTEXT pDevCtx );
static int DoGetNetDevTxChannel( PXTMRT_NETDEV_TXCHANNEL pParm );
static int bcmxtmrt_add_proc_files( void );
static int bcmxtmrt_del_proc_files( void );
static int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt, 
    int *eof, void *data);


/* Globals. */
static BCMXTMRT_GLOBAL_INFO g_GlobalInfo;
static struct atm_ioctl g_PppoAtmIoctlOps =
    {
        .ioctl    = bcmxtmrt_atm_ioctl,
    };
static struct ppp_channel_ops g_PppoAtmOps =
    {
        .start_xmit = bcmxtmrt_pppoatm_send
    };
static const struct atmdev_ops g_AtmDevOps =
    {
        .open       = bcmxtmrt_atmdev_open,
        .close      = bcmxtmrt_atmdev_close,
        .send       = bcmxtmrt_atmdev_send,
    };

/*
 *  Data integrity and cache management.
 *          - Prior to xmit, the modified portion of a data buffer that is
 *            to be transmitted needs to be wback flushed so that the data
 *            in the buffer is coherent with that in the L2 Cache.
 *            The region is demarcated by
 *                skb->data and skb->len
 *          - When a buffer is recycled, the buffer region that is handed to
 *            the DMA controller needs to be invalidated in L2 Cache.
 *            The region is demarcated by
 *                skb->head + RESERVED HEADROOM and skb->end + skb_shared_info
 *          - Given that the entire skb_shared_info is not accessed,
 *            e.g. the skb_frag_t array is only accessed if nr_frags is > 0,
 *            it is sufficient to only flush/invalidate a portion of the
 *            skb_shared_info that is placed after the skb->end.
 *
 *  Cache operations reworked so as to not perform any operation beyond
 *  the "end" demarcation.
 */

/*
 * Macros to round down and up, an address to a cachealigned address
 */
#define ROUNDDN(addr, align)  ( (addr) & ~((align) - 1) )
#define ROUNDUP(addr, align)  ( ((addr) + (align) - 1) & ~((align) - 1) )


/***************************************************************************
 * Function Name: cache_wbflush_region
 * Description  : Do MIPS flush cache operation on a buffer.
 *                if (addr == end) && (addr is cache aligned) no operation.
 *                if (addr == end) && (addr is not cache aligned) flush.
 *
 *                addr is rounded down to cache line.
 *                end is rounded up to cache line.
 *
 *                All cache lines from addr, NOT INCLUDING end are flushed. 
 * Returns      : None.
 ***************************************************************************/
static inline void cache_wbflush_region(void *addr, void *end)
{
    unsigned long dc_lsize = current_cpu_data.dcache.linesz;
    unsigned long a = ROUNDDN( (unsigned long)addr, dc_lsize );
    unsigned long e = ROUNDUP( (unsigned long)end, dc_lsize );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += dc_lsize;
    }
}


/***************************************************************************
 * Function Name: cache_wbflush_len
 * Description  : Do MIPS invalidate cache operation on a buffer.
 *                if (len == 0) && (addr is cache aligned) then no operation.
 *                if (len == 0) && (addr is not cache aligned) then flush !
 *                end = addr + len, then rounded up to cacheline
 *                addr is rounded down to cache line.
 *                All cache lines from addr, NOT INCLUDING end are flushed.
 * Returns      : None.
 ***************************************************************************/
static inline void cache_wbflush_len(void *addr, int len)
{
    unsigned long dc_lsize = current_cpu_data.dcache.linesz;
    unsigned long a = ROUNDDN( (unsigned long)addr, dc_lsize );
    unsigned long e = ROUNDUP( ((unsigned long)addr + len), dc_lsize );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += dc_lsize;
    }
}


/***************************************************************************
 * Function Name: bcmxtmrt_init
 * Description  : Called when the driver is loaded.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int __init bcmxtmrt_init( void )
{
    UINT16 usChipId  = (PERF->RevID & 0xFFFF0000) >> 16;
    UINT16 usChipRev = (PERF->RevID & 0xFF);

    printk(CARDNAME ": Broadcom BCM%X%X ATM/PTM Network Device ", usChipId,
        usChipRev);
    printk(VER_STR "\n");

    memset(&g_GlobalInfo, 0x00, sizeof(g_GlobalInfo));

    g_GlobalInfo.ulChipRev = PERF->RevID;
    register_atm_ioctl(&g_PppoAtmIoctlOps);
    g_GlobalInfo.pAtmDev = atm_dev_register("bcmxtmrt_atmdev", &g_AtmDevOps,
        -1, NULL);
    if( g_GlobalInfo.pAtmDev )
    {
        g_GlobalInfo.pAtmDev->ci_range.vpi_bits = 12;
        g_GlobalInfo.pAtmDev->ci_range.vci_bits = 16;
    }

    g_GlobalInfo.ulNumExtBufs = NR_RX_BDS(getMemorySize());
    g_GlobalInfo.ulNumExtBufsRsrvd = g_GlobalInfo.ulNumExtBufs / 5;
    g_GlobalInfo.ulNumExtBufs90Pct = (g_GlobalInfo.ulNumExtBufs * 9) / 10;
    g_GlobalInfo.ulNumExtBufs50Pct = g_GlobalInfo.ulNumExtBufs / 2;

    bcmxtmrt_add_proc_files();

    return( 0 );
} /* bcmxtmrt_init */


/***************************************************************************
 * Function Name: bcmxtmrt_cleanup
 * Description  : Called when the driver is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_cleanup( void )
{
    bcmxtmrt_del_proc_files();
    deregister_atm_ioctl(&g_PppoAtmIoctlOps);
    if( g_GlobalInfo.pAtmDev )
    {
        atm_dev_deregister( g_GlobalInfo.pAtmDev );
        g_GlobalInfo.pAtmDev = NULL;
    }
} /* bcmxtmrt_cleanup */


/***************************************************************************
 * Function Name: bcmxtmrt_open
 * Description  : Called to make the device operational.  Called due to shell
 *                command, "ifconfig <device_name> up".
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_open( struct net_device *dev )
{
    int nRet = 0;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;

    netif_start_queue(dev);

    if( pDevCtx->ulAdminStatus == ADMSTS_UP )
        pDevCtx->ulOpenState = XTMRT_DEV_OPENED;
    else
        nRet = -EIO;

    return( nRet );
} /* bcmxtmrt_open */


/***************************************************************************
 * Function Name: bcmxtmrt_close
 * Description  : Called to stop the device.  Called due to shell command,
 *                "ifconfig <device_name> down".
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_close( struct net_device *dev )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;

    pDevCtx->ulOpenState = XTMRT_DEV_CLOSED;
    netif_stop_queue(dev);
    return 0;
} /* bcmxtmrt_close */


/***************************************************************************
 * Function Name: bcmxtmrt_timeout
 * Description  : Called when there is a transmit timeout. 
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_timeout( struct net_device *dev )
{
    dev->trans_start = jiffies;
    netif_wake_queue(dev);
} /* bcmxtmrt_timeout */


/***************************************************************************
 * Function Name: bcmxtmrt_query
 * Description  : Called to return device statistics. 
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static struct net_device_stats *bcmxtmrt_query(struct net_device *dev)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
    struct net_device_stats *pStats = &pDevCtx->DevStats;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    if( pDevCtx->ucTxVcid != INVALID_VCID )
        pStats->tx_bytes += pGi->pulMibTxOctetCountBase[pDevCtx->ucTxVcid];

    for( i = 0; i < MAX_DEFAULT_MATCH_IDS; i++ )
    {
        if( pGi->pDevCtxsByMatchId[i] == pDevCtx )
        {
            *pGi->pulMibRxMatch = i;
            pStats->rx_bytes = *pGi->pulMibRxOctetCount;
            *pGi->pulMibRxMatch = i;
            pStats->rx_packets = *pGi->pulMibRxPacketCount;

            /* By convension, VCID 0 collects statistics for packets that use
             * Packet CMF rules.
             */
            if( i == 0 )
            {
                UINT32 j;
                for( j = MAX_DEFAULT_MATCH_IDS + 1; j < MAX_MATCH_IDS; j++ )
                {
                    *pGi->pulMibRxMatch = j;
                    pStats->rx_bytes += *pGi->pulMibRxOctetCount;
                    *pGi->pulMibRxMatch = j;
                    pStats->rx_packets += *pGi->pulMibRxPacketCount;
                }
            }
        }
    }

    return( pStats );
} /* bcmxtmrt_query */


/***************************************************************************
 * Function Name: bcmxtmrt_ioctl
 * Description  : Driver IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
    int *data=(int*)Req->ifr_data;
    int status;
    MirrorCfg mirrorCfg;
    int nRet = 0;

    switch (nCmd)
    {
    case SIOCGLINKSTATE:
        if( pDevCtx->ulLinkState == LINK_UP )
            status = LINKSTATE_UP;
        else
            status = LINKSTATE_DOWN;
        if (copy_to_user((void*)data, (void*)&status, sizeof(int)))
            nRet = -EFAULT;
        break;

    case SIOCSCLEARMIBCNTR:
        memset(&pDevCtx->DevStats, 0, sizeof(struct net_device_stats));
        break;

    case SIOCMIBINFO:
        if (copy_to_user((void*)data, (void*)&pDevCtx->MibInfo,
            sizeof(pDevCtx->MibInfo)))
        {
            nRet = -EFAULT;
        }
        break;

    case SIOCPORTMIRROR:
        if(copy_from_user((void*)&mirrorCfg,data,sizeof(MirrorCfg)))
            nRet=-EFAULT;
        else
        {
            if( mirrorCfg.nDirection == MIRROR_DIR_IN )
            {
                if( mirrorCfg.nStatus == MIRROR_ENABLED )
                    strcpy(pDevCtx->szMirrorIntfIn, mirrorCfg.szMirrorInterface);
                else
                    memset(pDevCtx->szMirrorIntfIn, 0x00, MIRROR_INTF_SIZE);
            }
            else /* MIRROR_DIR_OUT */
            {
                if( mirrorCfg.nStatus == MIRROR_ENABLED )
                    strcpy(pDevCtx->szMirrorIntfOut, mirrorCfg.szMirrorInterface);
                else
                    memset(pDevCtx->szMirrorIntfOut, 0x00, MIRROR_INTF_SIZE);
            }
        }
        break;

    case SIOCETHTOOL:
        nRet = bcmxtmrt_ethtool_ioctl(pDevCtx, (void *) Req->ifr_data);
        break;

    default:
        nRet = -EOPNOTSUPP;    
        break;
    }

    return( nRet );
} /* bcmxtmrt_ioctl */


/***************************************************************************
 * Function Name: bcmxtmrt_ethtool_ioctl
 * Description  : Driver ethtool IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *useraddr)
{
    struct ethtool_drvinfo info;
    struct ethtool_cmd ecmd;
    unsigned long ethcmd;
    int nRet = 0;

    if( copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)) == 0 )
    {
        switch (ethcmd)
        {
        case ETHTOOL_GDRVINFO:
            info.cmd = ETHTOOL_GDRVINFO;
            strncpy(info.driver, CARDNAME, sizeof(info.driver)-1);
            strncpy(info.version, VERSION, sizeof(info.version)-1);
            if (copy_to_user(useraddr, &info, sizeof(info)))
                nRet = -EFAULT;
            break;

        case ETHTOOL_GSET:
            ecmd.cmd = ETHTOOL_GSET;
            ecmd.speed = pDevCtx->MibInfo.ulIfSpeed / (1024 * 1024);
            if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
                nRet = -EFAULT;
            break;

        default:
            nRet = -EOPNOTSUPP;    
            break;
        }
    }
    else
       nRet = -EFAULT;

    return( nRet );
}

/***************************************************************************
 * Function Name: bcmxtmrt_atm_ioctl
 * Description  : Driver ethtool IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd,
    unsigned long arg)
{
    struct atm_vcc *pAtmVcc = ATM_SD(sock);
    void __user *argp = (void __user *)arg;
    atm_backend_t b;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    int nRet = -ENOIOCTLCMD;

    switch( cmd )
    {
    case ATM_SETBACKEND:
        if( get_user(b, (atm_backend_t __user *) argp) == 0 )
        {
            switch (b)
            {
            case ATM_BACKEND_PPP_BCM:
                if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci))!=NULL &&
                    pDevCtx->Chan.private == NULL )
                {
                    pDevCtx->Chan.private = pDevCtx->pDev;
                    pDevCtx->Chan.ops = &g_PppoAtmOps;
                    pDevCtx->Chan.mtu = 1500; /* TBD. Calc value. */
                    pAtmVcc->user_back = pDevCtx;
                    if( ppp_register_channel(&pDevCtx->Chan) == 0 )
                        nRet = 0;
                    else
                        nRet = -EFAULT;
                }
                else
                    nRet = (pDevCtx) ? 0 : -EFAULT;
                break;

            case ATM_BACKEND_PPP_BCM_DISCONN:
                /* This is a patch for PPP reconnection.
                 * ppp daemon wants us to send out an LCP termination request
                 * to let the BRAS ppp server terminate the old ppp connection.
                 */
                if((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
                {
                    struct sk_buff *skb;
                    int size = 6;
                    int eff  = (size+3) & ~3; /* align to word boundary */

                    while (!(skb = alloc_skb(eff, GFP_KERNEL)))
                        schedule();

                    skb->dev = NULL; /* for paths shared with net_device interfaces */
                    skb_put(skb, size);

                    skb->data[0] = 0xc0;  /* PPP_LCP == 0xc021 */
                    skb->data[1] = 0x21;
                    skb->data[2] = 0x05;  /* TERMREQ == 5 */
                    skb->data[3] = 0x02;  /* id == 2 */
                    skb->data[4] = 0x00;  /* HEADERLEN == 4 */
                    skb->data[5] = 0x04;

                    if (eff > size)
                        memset(skb->data+size,0,eff-size);

                    nRet = bcmxtmrt_xmit( skb, pDevCtx->pDev );
                }
                else
                    nRet = -EFAULT;
                break;

            case ATM_BACKEND_PPP_BCM_CLOSE_DEV:
                if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
                {
                    bcmxtmrt_pppoatm_send(&pDevCtx->Chan, NULL);
                    ppp_unregister_channel(&pDevCtx->Chan);
                    pDevCtx->Chan.private = NULL;
                }
                nRet = 0;
                break;

            default:
                break;
            }
        }
        else
            nRet = -EFAULT;
        break;

    case PPPIOCGCHAN:
        if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL )
        {
            nRet = put_user(ppp_channel_index(&pDevCtx->Chan),
                (int __user *) argp) ? -EFAULT : 0;
        }
        else
            nRet = -EFAULT;
        break;

    case PPPIOCGUNIT:
        if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL )
        {
            nRet = put_user(ppp_unit_number(&pDevCtx->Chan),
                (int __user *) argp) ? -EFAULT : 0;
        }
        else
            nRet = -EFAULT;
        break;
    default:
        break;
    }

    return( nRet );
} /* bcmxtmrt_atm_ioctl */


/***************************************************************************
 * Function Name: FindDevCtx
 * Description  : Finds a device context structure for a VCC.
 * Returns      : Pointer to a device context structure or NULL.
 ***************************************************************************/
static PBCMXTMRT_DEV_CONTEXT FindDevCtx( short vpi, int vci )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( (pDevCtx = pGi->pDevCtxs[i]) != NULL )
        {
            if( pDevCtx->Addr.u.Vcc.usVpi == vpi &&
                pDevCtx->Addr.u.Vcc.usVci == vci )
            {
                break;
            }

            pDevCtx = NULL;
        }
    }

    return( pDevCtx );
} /* FindDevCtx */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_open
 * Description  : ATM device open
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc)
{
    set_bit(ATM_VF_READY,&pVcc->flags);
    return( 0 );
} /* bcmxtmrt_atmdev_open */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_close
 * Description  : ATM device open
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc)
{
    clear_bit(ATM_VF_READY,&pVcc->flags);
    clear_bit(ATM_VF_ADDR,&pVcc->flags);
} /* bcmxtmrt_atmdev_close */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_send
 * Description  : send data
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = FindDevCtx( pVcc->vpi, pVcc->vci );
    int nRet;

    if( pDevCtx )
        nRet = bcmxtmrt_xmit( skb, pDevCtx->pDev );
    else
        nRet = -EIO;

    return( nRet );
} /* bcmxtmrt_atmdev_send */



/***************************************************************************
 * Function Name: bcmxtmrt_pppoatm_send
 * Description  : Called by the PPP driver to send data.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan, struct sk_buff *skb)
{
    bcmxtmrt_xmit( skb, (struct net_device *) pChan->private);
    return(1);
} /* bcmxtmrt_pppoatm_send */


#ifdef ATM_BONDING_TEST

const static char asmData [CELL_PAYLOAD_SIZE] =
   { 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
     0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
     0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
     0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x00, 0x28, 0x2e, 0x2f, 0x30, 0x31} ;

static void sendAsmOnPort (PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   int nRet ;
	XTMRT_CELL  C ;
	PXTMRT_CELL pC ;
	UINT32 sendPort ;

	pC = &C ;
   pDevCtx->asmFlag = 1 ;
	sendPort = pDevCtx->portAsm ;
	memset (pC, 0, sizeof (XTMRT_CELL)) ;
	memcpy (&pC->ConnAddr, &pDevCtx->Addr, sizeof (XTM_ADDR)) ;
   pC->ucCircuitType = CTYPE_ASM_P0 + sendPort ;
	pDevCtx->portAsm = (pDevCtx->portAsm + 1) & (MAX_PHY_PORTS-1) ;
   memcpy (pC->ucData, asmData, CELL_PAYLOAD_SIZE) ;
	nRet = DoSendCellReq (pDevCtx, pC) ;
   pDevCtx->asmFlag = 0 ;
}

static void sendSkbAsAsm (PBCMXTMRT_DEV_CONTEXT pDevCtx, struct sk_buff *skb)
{
   int nRet ;
	UINT32 sendPort ;
   UINT8 ucCircuitType ;

	pDevCtx->asmFlag = 1 ;
	sendPort = pDevCtx->portAsm ;
   ucCircuitType = CTYPE_ASM_P0 + sendPort ;

   switch (ucCircuitType) {

      case CTYPE_ASM_P0:
         skb->protocol = FSTAT_CT_ASM_P0;
         break;

      case CTYPE_ASM_P1:
         skb->protocol = FSTAT_CT_ASM_P1;
         break;

      case CTYPE_ASM_P2:
         skb->protocol = FSTAT_CT_ASM_P2;
         break;

      case CTYPE_ASM_P3:
         skb->protocol = FSTAT_CT_ASM_P3;
         break;
   }

   skb->protocol |= SKB_PROTO_ATM_CELL;

   skb->len = CELL_PAYLOAD_SIZE ;
   skb->data [42] = 0x00 ;
   skb->data [43] = 0x28 ;
	pDevCtx->portAsm = (pDevCtx->portAsm + 1) & (MAX_PHY_PORTS-1) ;
	pDevCtx->asmFlag = 0 ;
}
#endif

/***************************************************************************
 * Function Name: QueuePacket
 * Description  : Determines whether to queue a packet for transmission based
 *                on the number of total external (ie Ethernet) buffers and
 *                buffers already queued.
 * Returns      : 1 to queue packet, 0 to drop packet
 ***************************************************************************/
inline int QueuePacket( PBCMXTMRT_GLOBAL_INFO pGi, PTXQINFO pTqi )
{
    int nRet = 0; /* default to drop packet */

    if( pGi->ulNumTxQs == 1 )
    {
        /* One total transmit queue.  Allow up to 90% of external buffers to
         * be queued on this transmit queue.
         */
        if( pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs90Pct )
        {
            nRet = 1; /* queue packet */
            pGi->ulDbgQ1++;
        }
        else
            pGi->ulDbgD1++;
    }
    else
    {
        if(pGi->ulNumExtBufs - pGi->ulNumTxBufsQdAll > pGi->ulNumExtBufsRsrvd)
        {
            /* The available number of external buffers is greater than the
             * reserved value.  Allow up to 50% of external buffers to be
             * queued on this transmit queue.
             */
            if( pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs50Pct )
            {
                nRet = 1; /* queue packet */
                pGi->ulDbgQ2++;
            }
            else
                pGi->ulDbgD2++;
        }
        else
        {
            /* Divide the reserved number of external buffers evenly among all
             * of the transmit queues.
             */
            if(pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufsRsrvd / pGi->ulNumTxQs)
            {
                nRet = 1; /* queue packet */
                pGi->ulDbgQ3++;
            }
            else
                pGi->ulDbgD3++;
        }
    }

    return( nRet );
} /* QueuePacket */


/***************************************************************************
 * Function Name: bcmxtmrt_xmit
 * Description  : Check for transmitted packets to free and, if skb is
 *                non-NULL, transmit a packet.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_xmit( struct sk_buff *skb, struct net_device *dev)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
    PTXQINFO pTqi;
    UINT32 i;
    UINT8 rfc2684_type;

    local_bh_disable();

    if( pDevCtx->ulLinkState == LINK_UP )
    {
        /* Free packets that have been transmitted. */
        for(i=0, pTqi=pDevCtx->TxQInfos; i<pDevCtx->ulTxQInfosSize; i++,pTqi++)
        {
            while( pTqi->ulFreeBds < pTqi->ulQueueSize &&
                !(pTqi->pBds[pTqi->ulHead].status & DMA_OWN) )
            {
                if( pTqi->ppSkbs[pTqi->ulHead] != NULL )
                {
                    dev_kfree_skb_any(pTqi->ppSkbs[pTqi->ulHead]);
                    pTqi->ppSkbs[pTqi->ulHead] = NULL;
                    pTqi->pBds[pTqi->ulHead].address = 0;
                }

                if( !(pTqi->pBds[pTqi->ulHead].status & DMA_WRAP) )
                    pTqi->ulHead++;
                else
                    pTqi->ulHead = 0;

                pTqi->ulFreeBds++;
                pTqi->ulNumTxBufsQdOne--;
                pGi->ulNumTxBufsQdAll--;
            }
        }

        pTqi = NULL;


#ifdef ATM_BONDING_TEST
        if (pDevCtx->asmFlag == 0) {
#endif
        if (skb)
        {
            switch( pDevCtx->ulHdrType )
            {
            case HT_LLC_SNAP_ROUTE_IP:
            case HT_VC_MUX_IPOA:
                /* Packet is passed to driver with MAC header for IPoA. */
                skb_pull(skb, ETH_HLEN);
                break;

            default:
                if( skb->len < ETH_ZLEN &&
                    (skb->protocol & ~FSTAT_CT_MASK) != SKB_PROTO_ATM_CELL )
                {
                    struct sk_buff *skb2 = skb_copy_expand(skb, 0, ETH_ZLEN -
                        skb->len, GFP_ATOMIC);
                    if( skb2 )
                    {
                        dev_kfree_skb_any(skb);
                        skb = skb2;
                        memset(skb->tail, 0, ETH_ZLEN - skb->len);
                        skb_put(skb, ETH_ZLEN - skb->len);
                    }
                }
                break;
            }
        }
#ifdef ATM_BONDING_TEST
        }
#endif

#if defined (ATM_BONDING_TEST) || defined (PTM_BONDING_TEST)
        if(( skb && pDevCtx->ulTxQInfosSize ) &&
		 !(skb->data[0] & 0x01))
#else
        if( skb && pDevCtx->ulTxQInfosSize )
#endif
        {
            /* Find a transmit queue to send on. */
            UINT32 ulPort = 0;
            UINT32 ulPtmPriority = 0;

#ifdef CONFIG_NETFILTER
            /* bit 2-0 of the 32-bit nfmark is the subpriority (0 to 7) set by ebtables.
             * bit 3   of the 32-bit nfmark is the DSL latency, 0=PATH0, 1=PATH1
             * bit 4   of the 32-bit nfmark is the PTM priority, 0=LOW,  1=HIGH
             */
            UINT32 ulPriority = skb->mark & 0x07;

            ulPort = (skb->mark >> 3) & 0x1;  //DSL latency

            if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM)
               ulPtmPriority = (skb->mark >> 4) & 0x1;

            pTqi=pDevCtx->pTxPriorities[ulPtmPriority][ulPort][ulPriority];
        
            /* If a transmit queue was not found, use the existing highest priority queue
             * that had been configured with the default Ptm priority and DSL latency (port).
             */
            if (pTqi == NULL && ulPriority > 1)
            {
               UINT32 ulPtmPriorityDflt;
               UINT32 ulPortDflt;

               if (pDevCtx->pTxPriorities[0][0][0] == &pDevCtx->TxQInfos[0])
               {
                  ulPtmPriorityDflt = 0;
                  ulPortDflt        = 0;
               }
               else if (pDevCtx->pTxPriorities[0][1][0] == &pDevCtx->TxQInfos[0])
               {
                  ulPtmPriorityDflt = 0;
                  ulPortDflt        = 1;
               }
               else if (pDevCtx->pTxPriorities[1][0][0] == &pDevCtx->TxQInfos[0])
               {
                  ulPtmPriorityDflt = 1;
                  ulPortDflt        = 0;
               }
               else
               {
                  ulPtmPriorityDflt = 1;
                  ulPortDflt        = 1;
               }
               for (i = ulPriority - 1; pTqi == NULL && i >= 1; i--)
                  pTqi = pDevCtx->pTxPriorities[ulPtmPriorityDflt][ulPortDflt][i];
            }
#endif

            /* If a transmit queue was not found, use the first one. */
            if( pTqi == NULL )
               pTqi = &pDevCtx->TxQInfos[0];

#ifdef ATM_BONDING_TEST
            {
               int i ;
               if (pDevCtx->asmFlag == 0) {
                  
                  if ((pDevCtx->sendAsm % 100) == 0) {
#if 1
                     sendAsmOnPort (pDevCtx) ;
#else
                     sendSkbAsAsm (pDevCtx, skb);
#endif
                  }
                  pDevCtx->sendAsm += 1 ;
               }
            }
#endif

            if( pTqi->ulFreeBds && QueuePacket(pGi, pTqi) )
            {
                volatile DmaDesc *pBd;

                /* Decrement total BD count.  Increment global count. */
                pTqi->ulFreeBds--;
                pTqi->ulNumTxBufsQdOne++;
                pGi->ulNumTxBufsQdAll++;

                if( pDevCtx->szMirrorIntfOut[0] != '\0' &&
                    (skb->protocol & ~FSTAT_CT_MASK) != SKB_PROTO_ATM_CELL &&
                    (pDevCtx->ulHdrType ==  HT_PTM ||
                    pDevCtx->ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
                    pDevCtx->ulHdrType ==  HT_VC_MUX_ETHERNET) )
                {
                    MirrorPacket( skb, pDevCtx->szMirrorIntfOut );
                }

                if( (pDevCtx->ulFlags & CNI_HW_ADD_HEADER) == 0 &&
                    HT_LEN(pDevCtx->ulHdrType) != 0 &&
                    (skb->protocol & ~FSTAT_CT_MASK) != SKB_PROTO_ATM_CELL
#ifdef ATM_BONDING_TEST
                    &&
                    pDevCtx->asmFlag != 1
#endif
                    )
                {
                    rfc2684_type = HT_TYPE(pDevCtx->ulHdrType);
                }
                else
                    rfc2684_type = 0;

                /* Configure FlowCache stack and configure CMF. */
                blog_emit( skb, dev, (unsigned int)pTqi->ulDmaIndex,
                        rfc2684_type, pDevCtx->ulEncapType );

                if ( rfc2684_type )
                {
                    AddRfc2684Hdr( &skb, pDevCtx );
                }

#ifdef PTM_BONDING_TEST
                AddPTMBondingHeader (&skb, pDevCtx) ;
#endif

                if( pGi->ulChipRev == CHIP_REV_BCM6368B0 &&
                    ((UINT32) skb->data & 0x07) != 0 )
                {
                    /* 8 byte align */
                    UINT32 ulAdjust =
                        (((UINT32)skb->data+0x07) & ~0x07) - (UINT32) skb->data;
                    struct sk_buff *skb2 =
                        skb_copy_expand( skb, 0, ulAdjust, GFP_ATOMIC );
                    if( skb2 )
                    {
                        dev_kfree_skb_any(skb);
                        skb = skb2;
                    }
                }

                /*
                 * Prior to transmit, dirty data to be transmitted is flushed
                 * A cloned sk_buff may still have read access to the data buffer.
                 * When the buffer is returned to the Rx buffer pool, the region
                 * between the DMA assigned data and the end+sharedinfo needs to
                 * be invalidated, irrespective of whether it was also partially
                 * cleaned before xmit.
                 */

                /* pBd only sees skb->data, no need to start from skb->head */
                cache_wbflush_len(skb->data, skb->len);

                /* Track sent SKB, so we can release them later */
                pTqi->ppSkbs[pTqi->ulTail] = skb;

                pBd = &pTqi->pBds[pTqi->ulTail];
                pBd->address = (UINT32) VIRT_TO_PHY(skb->data);
                pBd->length  = skb->len;
                pBd->status  = (pBd->status&DMA_WRAP) | DMA_SOP | DMA_EOP |
                    pDevCtx->ucTxVcid;

                if( (skb->protocol & ~FSTAT_CT_MASK) == SKB_PROTO_ATM_CELL )
                {
                    pBd->status |= skb->protocol & FSTAT_CT_MASK;
                    if( (pDevCtx->ulFlags & CNI_USE_ALT_FSTAT) != 0 )
                    {
                        pBd->status |= FSTAT_MODE_COMMON;
                        pBd->status &= ~(FSTAT_COMMON_INS_HDR_EN |
                            FSTAT_COMMON_HDR_INDEX_MASK);
                    }
                }
                else
                    if( pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_ATM )
                    {
                        pBd->status |= FSTAT_CT_AAL5;
                        if( (pDevCtx->ulFlags & CNI_USE_ALT_FSTAT) != 0 )
                        {
                            pBd->status |= FSTAT_MODE_COMMON;
                            if(HT_LEN(pDevCtx->ulHdrType) != 0 &&
                               (pDevCtx->ulFlags & CNI_HW_ADD_HEADER) != 0)
                            {
                                pBd->status |= FSTAT_COMMON_INS_HDR_EN |
                                    ((HT_TYPE(pDevCtx->ulHdrType) - 1) <<
                                    FSTAT_COMMON_HDR_INDEX_SHIFT);
                            }
                            else
                            {
                                pBd->status &= ~(FSTAT_COMMON_INS_HDR_EN |
                                    FSTAT_COMMON_HDR_INDEX_MASK);
                            }
                        }
                    }
                    else
                        pBd->status |= FSTAT_CT_PTM | FSTAT_PTM_ENET_FCS |
                            FSTAT_PTM_CRC;

#ifdef ATM_BONDING_TEST
                {
	            UINT32 time ;
                   time = jiffies * (1000 / HZ) ;
                   //printk ("R%ld\n", time) ;
                }
#endif

                pBd->status |= DMA_OWN;

                /* advance BD pointer to next in the chain. */
                if( !(pBd->status & DMA_WRAP) )
                    pTqi->ulTail++;
                else
                    pTqi->ulTail = 0;

                /* Enable DMA for this channel */
                pTqi->pDma->cfg = DMA_ENABLE;

                /* Transmitted bytes are counted in hardware. */
                pDevCtx->DevStats.tx_packets++;
                pDevCtx->pDev->trans_start = jiffies;
            }
            else
            {
                /* Transmit queue is full.  Free the socket buffer.  Don't call
                 * netif_stop_queue because this device may use more than one
                 * queue.
                 */
                dev_kfree_skb_any(skb);
                pDevCtx->DevStats.tx_errors++;
            }
        }
        else
        {
            if( skb )
            {
                dev_kfree_skb_any(skb);
                pDevCtx->DevStats.tx_dropped++;
            }
        }
    }
    else
    {
        if( skb )
        {
            dev_kfree_skb_any(skb);
            pDevCtx->DevStats.tx_dropped++;
        }
    }

    local_bh_enable();

    return 0;
} /* bcmxtmrt_xmit */


#ifdef PTM_BONDING_TEST
/***************************************************************************
 * Function Name: AddPTMBondingHeader
 * Description  : Adds the PTM Bonding Tx header to a WAN packet before transmitting
 *                it.
 * Returns      : None.
 ***************************************************************************/
static void AddPTMBondingHeader (struct sk_buff **ppskb, PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   int minheadroom  = sizeof (PTMBondHeader) * 8 ;
   PTMBondHeader *pPtmBondHdr ;
   int count, index, size ;
   struct sk_buff *skb = *ppskb;
   int headroom = skb_headroom(skb);
   int skbOrigLen = skb->len ;

   if (headroom < minheadroom)
   {
      struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);

      dev_kfree_skb(skb);
      if (skb2 == NULL)
         skb = NULL;
      else
         skb = skb2;
   }

   if( skb )
   {
      int skblen;
      int fragmentSz ;

      skb_push(skb, minheadroom);
      pPtmBondHdr = skb->data ;
      memcpy (pPtmBondHdr, &g_GlobalInfo.ptmBondHdr [0], sizeof (PTMBondHeader) * 8) ;
      fragmentSz = g_GlobalInfo.fragmentSz ;
      count = skbOrigLen/fragmentSz ;
      //g_GlobalInfo.fragmentSz += 1 ;
      if (g_GlobalInfo.fragmentSz > 256) /* 512 is Max Fragment Sz */
         g_GlobalInfo.fragmentSz = 64 ;
      for (index = 0; index < count-1 ; index++) {
         pPtmBondHdr[index].FragSize = fragmentSz + 4 ; /* 2 bytes of frag hdr, 2 bytes of PTM CRC */
      }

      if (index != 8) {
         
         if ((skbOrigLen % fragmentSz) != 0) {
            pPtmBondHdr[index].FragSize = fragmentSz + (skbOrigLen % fragmentSz) + 4 + 4 /* 4 bytes of FCS, 4 bytes of frag info */ ;
            pPtmBondHdr[index].PktEop = 1 ;
         }
         else {
            pPtmBondHdr[index].PktEop = 1 ;
            pPtmBondHdr[index].FragSize += fragmentSz + 4 + 4 ;/* 4 bytes of FCS */
         }
      }
      else {
            pPtmBondHdr[index-1].PktEop = 1 ;
            pPtmBondHdr[index-1].FragSize += fragmentSz + 4 /* 4 bytes of FCS */ ;
      }

      skblen = skb->len;
   }

   *ppskb = skb;;
} /* AddPTMBondingHeader */
#endif

/***************************************************************************
 * Function Name: AddRfc2684Hdr
 * Description  : Adds the RFC2684 header to an ATM packet before transmitting
 *                it.
 * Returns      : None.
 ***************************************************************************/
static void AddRfc2684Hdr(struct sk_buff **ppskb, PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
    const UINT32 ulMinPktSize = 60;

    /*
     * If layout of ucHdrs changes, need to fix blog.c + pktCmfProto.c, sorry!
     */
    UINT8 ucHdrs[][16] =
        {{},
         {0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00},
         {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00},
         {0xFE, 0xFE, 0x03, 0xCF},
         {0x00, 0x00}};
    int minheadroom = HT_LEN(pDevCtx->ulHdrType);
    struct sk_buff *skb = *ppskb;
    int headroom = skb_headroom(skb);

    if (headroom < minheadroom)
    {
        struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);

        dev_kfree_skb_any(skb);
        if (skb2 == NULL)
            skb = NULL;
        else
            skb = skb2;
    }
    if( skb )
    {
        int skblen;

        skb_push(skb, minheadroom);
        memcpy(skb->data, ucHdrs[HT_TYPE(pDevCtx->ulHdrType)], minheadroom);
        skblen = skb->len;
        if (skblen < ulMinPktSize)
        {
            struct sk_buff *skb2=skb_copy_expand(skb, 0, ulMinPktSize -
                skb->len, GFP_ATOMIC);

            dev_kfree_skb_any(skb);
            if (skb2 == NULL)
                skb = NULL;
            else
            {
                skb = skb2;
                memset(skb->tail, 0, ulMinPktSize - skb->len);
                skb_put(skb, ulMinPktSize - skb->len);
            }
        }
    }

    *ppskb = skb;;
} /* AddRfc2684Hdr */


/***************************************************************************
 * Function Name: AssignRxBuffer
 * Description  : Put a data buffer back on to the receive BD ring. 
 * Returns      : None.
 ***************************************************************************/
static void AssignRxBuffer( PRXBDINFO pRxBdInfo, UINT8 *pucData, UINT8 *pEnd)
{
    volatile DmaDesc *pRxBd = pRxBdInfo->pBdTail;
    volatile DmaChannelCfg *pRxDma = pRxBdInfo->pDma;
    UINT16 usWrap = pRxBd->status & DMA_WRAP;

    /* DMA never sees RXBUF_HEAD_RESERVE at head of pucData */
    cache_wbflush_region(pucData + RXBUF_HEAD_RESERVE, pEnd);

    if( pRxBd->address == 0 )
    {
        pRxBd->address = VIRT_TO_PHY(pucData + RXBUF_HEAD_RESERVE);
        pRxBd->length  = RXBUF_SIZE;
        pRxBd->status = usWrap | DMA_OWN;

        if( usWrap )
            pRxBdInfo->pBdTail = pRxBdInfo->pBdBase;
        else
            pRxBdInfo->pBdTail++;
    }
    else
    {
        /* This should not happen. */
        printk(CARDNAME ": No place to put free buffer.\n");
    }

    /* Restart DMA in case the DMA ran out of descriptors, and
     * is currently shut down.
     */
    if( (pRxDma->intStat & DMA_NO_DESC) != 0 )
    {
        pRxDma->intStat = DMA_NO_DESC;
        pRxDma->cfg = DMA_ENABLE;
    }
}


/***************************************************************************
 * Function Name: bcmxtmrt_freeskbordata
 * Description  : Put socket buffer header back onto the free list or a data
 *                buffer back on to the BD ring. 
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_freeskbordata( PRXBDINFO pRxBdInfo, struct sk_buff *skb,
    unsigned nFlag )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    local_bh_disable();

    if( nFlag & RETFREEQ_SKB )
    {
        skb->retfreeq_context = pGi->pFreeRxSkbList;
        pGi->pFreeRxSkbList = skb;
    }
    else
    {
        /* Compute region to writeback flush invalidate */
        UINT8 * pEnd = skb->end + offsetof(struct skb_shared_info, frags);
        if ( unlikely( skb_shinfo(skb)->nr_frags ))
            pEnd += ( sizeof(skb_frag_t) * MAX_SKB_FRAGS );
        AssignRxBuffer( pRxBdInfo, skb->head, pEnd );
    }

    local_bh_enable();
} /* bcmxtmrt_freeskbordata */


/***************************************************************************
 * Function Name: bcmxtmrt_rxisr
 * Description  : Hardware interrupt that is called when a packet is received
 *                on one of the receive queues.
 * Returns      : IRQ_HANDLED
 ***************************************************************************/
static irqreturn_t bcmxtmrt_rxisr(int nIrq, void *pRxDma)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    UINT32 i;
    UINT32 ulScheduled = 0;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( (pDevCtx = pGi->pDevCtxs[i]) != NULL &&
            pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
        {
            /* Device is open.  Schedule the poll function. */
            netif_rx_schedule(pDevCtx->pDev);
            ((volatile DmaChannelCfg *) pRxDma)->intStat = DMA_DONE;
            pGi->ulIntEnableMask |=
                1 << (((UINT32) pRxDma - (UINT32)pGi->pRxDmaBase) /
                sizeof(DmaChannelCfg));
            ulScheduled = 1;
        }
    }

    if( ulScheduled == 0 )
    {
        /* Device is not open.  Reenable interrupt. */
        ((volatile DmaChannelCfg *) pRxDma)->intStat = DMA_DONE;
        BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + (((UINT32)pRxDma -
            (UINT32)pGi->pRxDmaBase) / sizeof(DmaChannelCfg)));
    }

    return( IRQ_HANDLED );
} /* bcmxtmrt_rxisr */


/***************************************************************************
 * Function Name: bcmxtmrt_poll
 * Description  : Hardware interrupt that is called when a packet is received
 *                on one of the receive queues.
 * Returns      : IRQ_HANDLED
 ***************************************************************************/
static int bcmxtmrt_poll(struct net_device * dev, int * budget)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 ulMask;
    UINT32 i;
    UINT32 work_to_do = min(dev->quota, *budget);
    UINT32 work_done;
    UINT32 ret_done;
    UINT32 more_to_do = 0;
    UINT32 flags;

    local_save_flags(flags);
    local_irq_disable();
    ulMask = pGi->ulIntEnableMask;
    pGi->ulIntEnableMask = 0;
    local_irq_restore(flags);

    work_done = bcmxtmrt_rxtask(work_to_do, &more_to_do);
    ret_done = work_done & XTM_POLL_DONE;
    work_done &= ~XTM_POLL_DONE;

    *budget -= work_done;
    dev->quota -= work_done;

    if (work_done < work_to_do && ret_done != XTM_POLL_DONE)
    {
        /* Did as much as could, but we are not done yet */
        local_save_flags(flags);
        local_irq_disable();
        pGi->ulIntEnableMask |= ulMask;
        local_irq_restore(flags);
        return 1;
    }

    /* We are done */
    netif_rx_complete(dev);

    /* Renable interrupts. */
    for( i = 0; ulMask && i < MAX_RECEIVE_QUEUES; i++, ulMask >>= 1 )
        if( (ulMask & 0x01) == 0x01 )
            BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);

    return 0;
} /* bcmxtmrt_poll */


/***************************************************************************
 * Function Name: bcmxtmrt_rxtask
 * Description  : Linux Tasklet that processes received packets.
 * Returns      : None.
 ***************************************************************************/
static UINT32 bcmxtmrt_rxtask( UINT32 ulBudget, UINT32 *pulMoreToDo )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 ulMoreToReceive;
    UINT32 i;
    UINT8 rfc2684_type;
    PRXBDINFO pRxBdInfo;
    volatile DmaDesc *pRxBd;
    struct sk_buff *skb;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    UINT32 ulRxPktGood = 0;
    UINT32 ulRxPktProcessed = 0;
    UINT32 ulRxPktMax = ulBudget + (ulBudget / 2);

    /* Receive packets from every receive queue in a round robin order until
     * there are no more packets to receive.
     */
    do
    {
        ulMoreToReceive = 0;

        for( i = 0, pRxBdInfo = pGi->RxBdInfos; i < MAX_RECEIVE_QUEUES;
             i++, pRxBdInfo++ )
        {
            pRxBd = pRxBdInfo->pBdHead;

            if( pRxBd && !(pRxBd->status & DMA_OWN) && pRxBd->address )
            {
                UINT8 *pucData = (UINT8 *) KSEG0ADDR(pRxBd->address);
                UINT16 usStatus = pRxBd->status;

                if( ulBudget == 0 )
                {
                    *pulMoreToDo = 1;
                    break;
                }

                ulRxPktProcessed++;

                pRxBd->address = 0;
                pRxBd->status &= DMA_WRAP;

                /* Advance head BD pointer for this receive BD ring. */
                if( (usStatus & DMA_WRAP) != 0 )
                    pRxBdInfo->pBdHead = pRxBdInfo->pBdBase;
                else
                    pRxBdInfo->pBdHead++;

                pDevCtx = pGi->pDevCtxsByMatchId[usStatus&FSTAT_MATCH_ID_MASK];

                if( (usStatus & FSTAT_ERROR) == 0 && (pDevCtx ||
                     (usStatus & FSTAT_PACKET_CELL_MASK) == FSTAT_CELL) )
                {
                    if((usStatus & FSTAT_PACKET_CELL_MASK) == FSTAT_PACKET)
                    {
                        ulRxPktGood++;

                        if( (pDevCtx->ulFlags & LSC_RAW_ENET_MODE) != 0 )
                            pRxBd->length -= 4; /* substract Ethernet CRC */

                        /* Get an skb to return to the network stack. */
                        skb = pGi->pFreeRxSkbList;
                        pGi->pFreeRxSkbList =
                            pGi->pFreeRxSkbList->retfreeq_context;

                        if( pRxBd->length < ETH_ZLEN )
                            pRxBd->length = ETH_ZLEN;

#ifdef ATM_BONDING_TEST
            {
	            UINT32 time ;
               time = jiffies * (1000 / HZ) ;
               //printk ("S%ld\n", time) ;
            }
#endif

                        skb_hdrinit(RXBUF_HEAD_RESERVE, pRxBd->length +
                            SAR_DMA_MAX_BURST_LENGTH, skb, pucData, (void *)
                            bcmxtmrt_freeskbordata, pRxBdInfo, FROM_WAN);

                        __skb_trim(skb, pRxBd->length);

                        rfc2684_type = 0;
                        if( (pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0 )
                        {
                            if ( HT_LEN(pDevCtx->ulHdrType) > 0 )
                            {
                                rfc2684_type = HT_TYPE(pDevCtx->ulHdrType);
                                __skb_pull(skb, HT_LEN(pDevCtx->ulHdrType));
                            }
                        }

                        skb->dev = pDevCtx->pDev;

                        if( pDevCtx->szMirrorIntfIn[0] != '\0' &&
                            (pDevCtx->ulHdrType ==  HT_PTM ||
                            pDevCtx->ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
                            pDevCtx->ulHdrType ==  HT_VC_MUX_ETHERNET) )
                        {
                            MirrorPacket( skb, pDevCtx->szMirrorIntfIn );
                        }

                        /* CONFIG_BLOG */
                        if ( blog_init( skb, skb->dev,
                                        (usStatus & FSTAT_MATCH_ID_MASK),
                                        rfc2684_type,
                                        pDevCtx->ulEncapType ) == PKT_DONE )
                        {   /* if packet consumed, proceed to next packet */
                            pDevCtx->pDev->last_rx = jiffies;
                            ulMoreToReceive = 1;
                            ulBudget--;
                            continue;   /* next packet */
                        }

                        pDevCtx->pDev->last_rx = jiffies;

                        switch( pDevCtx->ulHdrType )
                        {
                        case HT_LLC_SNAP_ROUTE_IP:
                        case HT_VC_MUX_IPOA:
                            /* IPoA */
                            skb->protocol = htons(ETH_P_IP);
                            skb->mac.raw = skb->data;

                            /* Give the received packet to the network stack. */
                            pDevCtx->DevStats.rx_bytes += skb->len;
                            netif_receive_skb(skb);
                            break;

                        case HT_LLC_ENCAPS_PPP:
                        case HT_VC_MUX_PPPOA:
                            /*PPPoA*/
                            pDevCtx->DevStats.rx_bytes += skb->len;
                            ppp_input(&pDevCtx->Chan, skb);
                            break;

                        default:
                            /* bridge, MER, PPPoE */
                            if( (skb->data[0] & 0x01) == 0x01 )
                                pDevCtx->DevStats.multicast++;
                            skb->protocol = eth_type_trans(skb,pDevCtx->pDev);

                            /* Give the received packet to the network stack. */
                            pDevCtx->DevStats.rx_bytes += skb->len;
                            netif_receive_skb(skb);
                            break;
                        }
                        ulBudget--;
                    }
                    else /* cell */
                    {
                        ProcessRxCell( pGi, pRxBdInfo, pucData );
                        if( ulRxPktProcessed >= ulRxPktMax )
                            break;
                    }
                }
                else /* packet error */
                {
                    /* No cache operation as no data was accessed */
                    AssignRxBuffer(pRxBdInfo, pucData - RXBUF_HEAD_RESERVE,
                                              pucData - RXBUF_HEAD_RESERVE);

                    if( pDevCtx )
                        pDevCtx->DevStats.rx_errors++;

                    if( ulRxPktProcessed >= ulRxPktMax )
                        break;
                }

                /* There may be more packets to receive on this Rx queue. */
                ulMoreToReceive = 1;
            }
            else
                ulRxPktGood |= XTM_POLL_DONE;
        }
    } while( ulMoreToReceive );

    return( ulRxPktGood );
} /* bcmxtmrt_rxtask */


/***************************************************************************
 * Function Name: ProcessRxCell
 * Description  : Processes a received cell.
 * Returns      : None.
 ***************************************************************************/
static void ProcessRxCell( PBCMXTMRT_GLOBAL_INFO pGi, PRXBDINFO pRxBdInfo,
    UINT8 *pucData )
{
    const UINT16 usOamF4VciSeg = 3;
    const UINT16 usOamF4VciEnd = 4;
    UINT8 ucCts[] = {0, 0, 0, 0, CTYPE_OAM_F5_SEGMENT, CTYPE_OAM_F5_END_TO_END,
        0, 0, CTYPE_ASM_P0, CTYPE_ASM_P1, CTYPE_ASM_P2, CTYPE_ASM_P3,
        CTYPE_OAM_F4_SEGMENT, CTYPE_OAM_F4_END_TO_END};
    XTMRT_CELL Cell;
    UINT8 ucCHdr = *pucData;
    UINT8 *pucAtmHdr = pucData + sizeof(char);
    UINT8 ucLogPort;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;

    /* Fill in the XTMRT_CELL structure */
    Cell.ConnAddr.ulTrafficType = TRAFFIC_TYPE_ATM;
    ucLogPort = PORT_PHYS_TO_LOG((ucCHdr & CHDR_PORT_MASK) >> CHDR_PORT_SHIFT);
    Cell.ConnAddr.u.Vcc.ulPortMask = PORT_TO_PORTID(ucLogPort);
    Cell.ConnAddr.u.Vcc.usVpi = (((UINT16) pucAtmHdr[0] << 8) +
        ((UINT16) pucAtmHdr[1])) >> 4;
    Cell.ConnAddr.u.Vcc.usVci = (UINT16)
        (((UINT32) (pucAtmHdr[1] & 0x0f) << 16) +
         ((UINT32) pucAtmHdr[2] << 8) +
         ((UINT32) pucAtmHdr[3])) >> 4;

    if( Cell.ConnAddr.u.Vcc.usVci == usOamF4VciSeg )
    {
        ucCHdr = CHDR_CT_OAM_F4_SEG;
        pDevCtx = pGi->pDevCtxs[0];
    }
    else
        if( Cell.ConnAddr.u.Vcc.usVci == usOamF4VciEnd )
        {
            ucCHdr = CHDR_CT_OAM_F4_E2E;
            pDevCtx = pGi->pDevCtxs[0];
        }
        else
        {
            pDevCtx = FindDevCtx( (short) Cell.ConnAddr.u.Vcc.usVpi,
                (int) Cell.ConnAddr.u.Vcc.usVci);
        }

    Cell.ucCircuitType = ucCts[(ucCHdr & CHDR_CT_MASK) >> CHDR_CT_SHIFT];

    if( (ucCHdr & CHDR_ERROR) == 0 )
    {
        memcpy(Cell.ucData, pucData + sizeof(char), sizeof(Cell.ucData));

        /* Call the registered OAM or ASM callback function. */
        switch( ucCHdr & CHDR_CT_MASK )
        {
        case CHDR_CT_OAM_F5_SEG:
        case CHDR_CT_OAM_F5_E2E:
        case CHDR_CT_OAM_F4_SEG:
        case CHDR_CT_OAM_F4_E2E:
            if( pGi->pfnOamHandler && pDevCtx )
            {
                (*pGi->pfnOamHandler) ((XTMRT_HANDLE)pDevCtx,
                    XTMRTCB_CMD_CELL_RECEIVED, &Cell,
                    pGi->pOamContext);
            }
            break;

        case CHDR_CT_ASM_P0:
        case CHDR_CT_ASM_P1:
        case CHDR_CT_ASM_P2:
        case CHDR_CT_ASM_P3:
            if( pGi->pfnAsmHandler && pDevCtx )
            {
                (*pGi->pfnAsmHandler) ((XTMRT_HANDLE)pDevCtx,
                    XTMRTCB_CMD_CELL_RECEIVED, &Cell,
                    pGi->pAsmContext);
            }
#if 0
#ifdef ATM_BONDING_TEST
				printk ("Received ASM CT = %x \n", ucCHdr&CHDR_CT_MASK) ;
#endif
#endif
            break;

        default:
            break;
        }
    }
    else
        if( pDevCtx )
            pDevCtx->DevStats.rx_errors++;

    /* Put the buffer back onto the BD ring. */
    /* Compute region to writeback flush invalidate */
    AssignRxBuffer(pRxBdInfo, pucData - RXBUF_HEAD_RESERVE,
                              pucData - RXBUF_HEAD_RESERVE + RXBUF_SIZE);

} /* ProcessRxCell */

/***************************************************************************
 * Function Name: MirrorPacket
 * Description  : This function sends a sent or received packet to a LAN port.
 *                The purpose is to allow packets sent and received on the WAN
 *                to be captured by a protocol analyzer on the Lan for debugging
 *                purposes.
 * Returns      : None.
 ***************************************************************************/
static void MirrorPacket( struct sk_buff *skb, char *intfName )
{
    struct sk_buff *skbClone;
    struct net_device *netDev;

    if( (skbClone = skb_clone(skb, GFP_ATOMIC)) != NULL )
    {
        if( (netDev = __dev_get_by_name(intfName)) != NULL )
        {
            unsigned long flags;

            blog_xfer(skb, skbClone);
            skbClone->dev = netDev;
            skbClone->protocol = htons(ETH_P_802_3);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(skbClone) ;
            local_irq_restore(flags);
        }
        else
            dev_kfree_skb(skbClone);
    }
} /* MirrorPacket */

/***************************************************************************
 * Function Name: bcmxtmrt_timer
 * Description  : Periodic timer that calls the send function to free packets
 *                that have been transmitted.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_timer( PBCMXTMRT_GLOBAL_INFO pGi )
{
    UINT32 i;

    /* Free transmitted buffers. */
    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] )
            bcmxtmrt_xmit( NULL, pGi->pDevCtxs[i]->pDev );

    /* Restart the timer. */
    pGi->Timer.expires = jiffies + SAR_TIMEOUT;
    add_timer(&pGi->Timer);
} /* bcmxtmrt_timer */


/***************************************************************************
 * Function Name: bcmxtmrt_request
 * Description  : Request from the bcmxtmcfg driver.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int bcmxtmrt_request( XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParm )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = (PBCMXTMRT_DEV_CONTEXT) hDev;
    int nRet = 0;

    switch( ulCommand )
    {
    case XTMRT_CMD_GLOBAL_INITIALIZATION:
        nRet = DoGlobInitReq( (PXTMRT_GLOBAL_INIT_PARMS) pParm );
        break;

    case XTMRT_CMD_CREATE_DEVICE:
        nRet = DoCreateDeviceReq( (PXTMRT_CREATE_NETWORK_DEVICE) pParm );
        break;

    case XTMRT_CMD_GET_DEVICE_STATE:
        *(UINT32 *) pParm = pDevCtx->ulOpenState;
        break;

    case XTMRT_CMD_SET_ADMIN_STATUS:
        pDevCtx->ulAdminStatus = (UINT32) pParm;
        break;

    case XTMRT_CMD_REGISTER_CELL_HANDLER:
        nRet = DoRegCellHdlrReq( (PXTMRT_CELL_HDLR) pParm );
        break;

    case XTMRT_CMD_UNREGISTER_CELL_HANDLER:
        nRet = DoUnregCellHdlrReq( (PXTMRT_CELL_HDLR) pParm );
        break;

    case XTMRT_CMD_LINK_STATUS_CHANGED:
        nRet = DoLinkStsChangedReq(pDevCtx, (PXTMRT_LINK_STATUS_CHANGE)pParm);
        break;

    case XTMRT_CMD_SEND_CELL:
        nRet = DoSendCellReq( pDevCtx, (PXTMRT_CELL) pParm );
        break;

    case XTMRT_CMD_DELETE_DEVICE:
        nRet = DoDeleteDeviceReq( pDevCtx );
        break;

    case XTMRT_CMD_SET_TX_QUEUE:
        nRet = DoSetTxQueue( pDevCtx, (PXTMRT_TRANSMIT_QUEUE_ID) pParm );
        break;

    case XTMRT_CMD_UNSET_TX_QUEUE:
        nRet = DoUnsetTxQueue( pDevCtx, (PXTMRT_TRANSMIT_QUEUE_ID) pParm );
        break;

    case XTMRT_CMD_GET_NETDEV_TXCHANNEL:
        nRet = DoGetNetDevTxChannel( (PXTMRT_NETDEV_TXCHANNEL) pParm );
        break;

    default:
        nRet = -EINVAL;
        break;
    }

    return( nRet );
} /* bcmxtmrt_request */


/***************************************************************************
 * Function Name: DoGlobInitReq
 * Description  : Processes an XTMRT_CMD_GLOBAL_INITIALIZATION command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoGlobInitReq( PXTMRT_GLOBAL_INIT_PARMS pGip )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    struct DmaDesc *pBd, *pBdBase;
    UINT32 ulBufsToAlloc;
    UINT32 ulAllocAmt;
    UINT8 *p;
    UINT32 i, j, ulSize;

    /* Save MIB counter registers. */
    pGi->pulMibTxOctetCountBase = pGip->pulMibTxOctetCountBase;
    pGi->pulMibRxMatch = pGip->pulMibRxMatch;
    pGi->pulMibRxOctetCount = pGip->pulMibRxOctetCount;
    pGi->pulMibRxPacketCount = pGip->pulMibRxPacketCount;

    /* Determine the number of receive buffers. */
    for( i = 0, ulBufsToAlloc = 0; i < MAX_RECEIVE_QUEUES; i++ )
        ulBufsToAlloc += pGip->ulReceiveQueueSizes[i];

    if( pGi->ulDrvState != XTMRT_UNINITIALIZED )
        nRet = -EPERM;

    /* Allocate receive DMA buffer descriptors. */
    ulSize = (ulBufsToAlloc * sizeof(struct DmaDesc)) + 0x10;
    if( nRet == 0 && (pGi->pRxBdMem = (struct DmaDesc *)
        kmalloc(ulSize, GFP_KERNEL)) != NULL )
    {
        pBdBase = (struct DmaDesc *) (((UINT32) pGi->pRxBdMem + 0x0f) & ~0x0f);
        p = (UINT8 *) pGi->pRxBdMem;
        cache_wbflush_len(p, ulSize);
        pBdBase = (struct DmaDesc *) CACHE_TO_NONCACHE(pBdBase);
    }
    else
        nRet = -ENOMEM;

    /* Allocate receive socket buffers and data buffers. */
    if( nRet == 0 )
    {
        const UINT32 ulRxAllocSize = SKB_ALIGNED_SIZE + RXBUF_ALLOC_SIZE;
        const UINT32 ulBlockSize = (64 * 1024);
        const UINT32 ulBufsPerBlock = ulBlockSize / ulRxAllocSize;

        /* Allocate one additional socket buffer so the socket buffer chain is
         * never empty.
         */
        if( (p = kmalloc(SKB_ALIGNED_SIZE, GFP_KERNEL)) != NULL )
        {
            memset(p, 0x00, SKB_ALIGNED_SIZE);
            ((struct sk_buff *) p)->retfreeq_context = pGi->pFreeRxSkbList;
            pGi->pFreeRxSkbList = (struct sk_buff *) p;
        }

        j = 0;
        pBd = pBdBase;
        while( ulBufsToAlloc )
        {
            ulAllocAmt = (ulBufsPerBlock < ulBufsToAlloc)
                ? ulBufsPerBlock : ulBufsToAlloc;

            ulSize = ulAllocAmt * ulRxAllocSize;
            if( j < MAX_BUFMEM_BLOCKS &&
                (p = kmalloc(ulSize, GFP_KERNEL)) != NULL )
            {
                UINT8 *p2;

                memset(p, 0x00, ulSize);
                pGi->pBufMem[j++] = p;
                cache_wbflush_len(p, ulSize);
                p = (UINT8 *) (((UINT32) p + 0x0f) & ~0x0f);
                for( i = 0; i < ulAllocAmt; i++ )
                {
                    pBd->status = DMA_OWN;
                    pBd->length = RXBUF_SIZE;
                    pBd->address = (UINT32)VIRT_TO_PHY(p + RXBUF_HEAD_RESERVE);
                    pBd++;

                    p2 = p + RXBUF_ALLOC_SIZE;
                    ((struct sk_buff *) p2)->retfreeq_context =
                        pGi->pFreeRxSkbList;
                    pGi->pFreeRxSkbList = (struct sk_buff *) p2;

                    p += ulRxAllocSize;
                }
                ulBufsToAlloc -= ulAllocAmt;
            }
            else
            {
                /* Allocation error. */
                for (i = 0; i < MAX_BUFMEM_BLOCKS; i++)
                {
                    if (pGi->pBufMem[i])
                    {
                        kfree(pGi->pBufMem[i]);
                        pGi->pBufMem[i] = NULL;
                    }
                }
                kfree((const UINT8 *) pGi->pRxBdMem);
                pGi->pRxBdMem = NULL;
                nRet = -ENOMEM;
            }
        }
    }

#ifdef PTM_BONDING_TEST
    /* Initialize PTM Bonding Header information */
    {
       int count ;

       for (count = 0 ; count < 8 ; count++) {
         pGi->ptmBondHdr [count].LineSel = count & 0x1 ;
         //pGi->ptmBondHdr [count].LineSel = 1 ;
       }

       printk ("Printing Initial value of ptmBondHdr \n" );
       dumpaddr (pGi->ptmBondHdr, 16) ;

       pGi->fragmentSz = 128 ;
    }

    printk ("<<< pGi->PhyDump [0] = %x >>>\n", pGi->PhyDump[0]) ;
    printk ("<<< pGi->PhyDump [1] = %x >>>\n", pGi->PhyDump[1]) ;
#endif

    /* Initialize receive DMA registers. */
    if( nRet == 0 )
    {
        volatile DmaChannelCfg *pRxDma, *pTxDma;
        volatile DmaStateRam *pStRam;

        /* Clear State RAM for receive DMA Channels. */
        pGi->pDmaRegs = (DmaRegs *) SAR_DMA_BASE;
        pStRam = pGi->pDmaRegs->stram.s;
        memset((char *) &pStRam[SAR_RX_DMA_BASE_CHAN], 0x00,
            sizeof(DmaStateRam) * NR_SAR_RX_DMA_CHANS);
        pGi->pRxDmaBase = pGi->pDmaRegs->chcfg+SAR_RX_DMA_BASE_CHAN;
        pRxDma = pGi->pRxDmaBase;
        pBd = pBdBase;

        for(i=0, pRxDma=pGi->pRxDmaBase; i < MAX_RECEIVE_QUEUES; i++, pRxDma++)
        {
            pRxDma->cfg = 0;
            BcmHalInterruptDisable(SAR_RX_INT_ID_BASE + i);
            if( pGip->ulReceiveQueueSizes[i] )
            {
                PRXBDINFO pRxBdInfo = &pGi->RxBdInfos[i];

                pRxDma->maxBurst = SAR_DMA_MAX_BURST_LENGTH;
                pRxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
                pRxDma->intMask = DMA_DONE;
                pRxBdInfo->pBdBase = pBd;
                pRxBdInfo->pBdHead = pBd;
                pRxBdInfo->pBdTail = pBd;
                pRxBdInfo->pDma = pRxDma;
                pStRam[SAR_RX_DMA_BASE_CHAN+i].baseDescPtr = (UINT32)
                    VIRT_TO_PHY(pRxBdInfo->pBdBase);
                pBd += pGip->ulReceiveQueueSizes[i] - 1;
                pBd++->status |= DMA_WRAP;

                BcmHalMapInterrupt((FN_HANDLER) bcmxtmrt_rxisr, (UINT32)
                    pRxDma, SAR_RX_INT_ID_BASE + i);
            }
            else
                memset(&pGi->RxBdInfos[i], 0x00, sizeof(RXBDINFO));
        }

        pGi->pDmaRegs->controller_cfg |= DMA_MASTER_EN;

        /* Clear State RAM for transmit DMA Channels. */
        memset( (char *) &pStRam[SAR_TX_DMA_BASE_CHAN], 0x00,
            sizeof(DmaStateRam) * NR_SAR_TX_DMA_CHANS );
        pGi->pTxDmaBase = pGi->pDmaRegs->chcfg+SAR_TX_DMA_BASE_CHAN;

        for(i=0, pTxDma=pGi->pTxDmaBase; i < MAX_TRANSMIT_QUEUES; i++, pTxDma++)
        {
            pTxDma->cfg = 0;
            BcmHalInterruptDisable(SAR_TX_INT_ID_BASE + i);
        }

        /* Initialize a timer function to free transmit buffers. */
        init_timer(&pGi->Timer);
        pGi->Timer.data = (unsigned long) pGi;
        pGi->Timer.function = (void *) bcmxtmrt_timer;

        pGi->ulDrvState = XTMRT_INITIALIZED;
    }

    return( nRet );
} /* DoGlobInitReq */


/***************************************************************************
 * Function Name: DoCreateDeviceReq
 * Description  : Processes an XTMRT_CMD_CREATE_DEVICE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoCreateDeviceReq( PXTMRT_CREATE_NETWORK_DEVICE pCnd )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    struct net_device *dev = NULL;

    if( pGi->ulDrvState != XTMRT_UNINITIALIZED &&
        (dev = alloc_netdev( sizeof(BCMXTMRT_DEV_CONTEXT),
         pCnd->szNetworkDeviceName, ether_setup )) != NULL )
    {
        dev_alloc_name(dev, dev->name);
        SET_MODULE_OWNER(dev);

        pDevCtx = (PBCMXTMRT_DEV_CONTEXT) dev->priv;
        memset(pDevCtx, 0x00, sizeof(BCMXTMRT_DEV_CONTEXT));
        memcpy(&pDevCtx->Addr, &pCnd->ConnAddr, sizeof(XTM_ADDR));
        if( pCnd->ConnAddr.ulTrafficType == TRAFFIC_TYPE_ATM )
            pDevCtx->ulHdrType = pCnd->ulHeaderType;
        else
            pDevCtx->ulHdrType = HT_PTM;
        pDevCtx->ulFlags = pCnd->ulFlags;
        pDevCtx->pDev = dev;
        pDevCtx->ulAdminStatus = ADMSTS_UP;
        pDevCtx->ucTxVcid = INVALID_VCID;

        /* Read and display the MAC address. */
        dev->dev_addr[0] = 0xff;
        kerSysGetMacAddress( dev->dev_addr, ((UINT32) pDevCtx & 0x00ffffff) |
            0x10000000 );
        if( (dev->dev_addr[0] & 0x01) == 0x01 )
        {
            printk( KERN_ERR CARDNAME": Unable to read MAC address from "
                "persistent storage.  Using default address.\n" );
            memcpy( dev->dev_addr, "\x02\x10\x18\x02\x00\x01", 6 );
        }

        printk( CARDNAME": MAC address: %2.2x %2.2x %2.2x %2.2x %2.2x "
            "%2.2x\n", dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
            dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5] );

        /* Setup the callback functions. */
        dev->open               = bcmxtmrt_open;
        dev->stop               = bcmxtmrt_close;
        dev->hard_start_xmit    = bcmxtmrt_xmit;
        dev->tx_timeout         = bcmxtmrt_timeout;
        dev->watchdog_timeo     = SAR_TIMEOUT;
        dev->get_stats          = bcmxtmrt_query;
        dev->set_multicast_list = NULL;
        dev->do_ioctl           = &bcmxtmrt_ioctl;
        dev->poll               = bcmxtmrt_poll;
        dev->weight             = 64;

        /* identify as a WAN interface to block WAN-WAN traffic */
        dev->priv_flags |= IFF_WANDEV;

        switch( pDevCtx->ulHdrType )
        {
        case HT_LLC_SNAP_ROUTE_IP:
        case HT_VC_MUX_IPOA:
            pDevCtx->ulEncapType = TYPE_IP;     /* IPoA */
            dev->type = ARPHRD_PPP;
            dev->hard_header_len = HT_LEN_LLC_SNAP_ROUTE_IP;
            dev->mtu = RFC1626_MTU;
            dev->addr_len = 0;
            dev->tx_queue_len = 100;
            dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
            break;

        case HT_LLC_ENCAPS_PPP:
        case HT_VC_MUX_PPPOA:
            pDevCtx->ulEncapType = TYPE_PPP;    /*PPPoA*/
            break;

        default:
            pDevCtx->ulEncapType = TYPE_ETH;    /* bridge, MER, PPPoE, PTM */
            dev->flags = IFF_MULTICAST;
            break;
        }
        
      
        /* Don't reset or enable the device yet. "Open" does that. */
        nRet = register_netdev(dev);
        if (nRet == 0) 
        {
            UINT32 i;
            for( i = 0; i < MAX_DEV_CTXS; i++ )
                if( pGi->pDevCtxs[i] == NULL )
                {
                    pGi->pDevCtxs[i] = pDevCtx;
                    break;
                }

            pCnd->hDev = (XTMRT_HANDLE) pDevCtx;
        }
        else
        {
            printk(KERN_ERR CARDNAME": register_netdev failed\n");
            free_netdev(dev);
        }

        if( nRet != 0 )
            kfree(pDevCtx);
    }
    else
    {
        printk(KERN_ERR CARDNAME": alloc_netdev failed\n");
        nRet = -ENOMEM;
    }

    return( nRet );
} /* DoCreateDeviceReq */


/***************************************************************************
 * Function Name: DoRegCellHdlrReq
 * Description  : Processes an XTMRT_CMD_REGISTER_CELL_HANDLER command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoRegCellHdlrReq( PXTMRT_CELL_HDLR pCh )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    switch( pCh->ulCellHandlerType )
    {
    case CELL_HDLR_OAM:
        if( pGi->pfnOamHandler == NULL )
        {
            pGi->pfnOamHandler = pCh->pfnCellHandler;
            pGi->pOamContext = pCh->pContext;
        }
        else
            nRet = -EEXIST;
        break;

    case CELL_HDLR_ASM:
        if( pGi->pfnAsmHandler == NULL )
        {
            pGi->pfnAsmHandler = pCh->pfnCellHandler;
            pGi->pAsmContext = pCh->pContext;
        }
        else
            nRet = -EEXIST;
        break;
    }

    return( nRet );
} /* DoRegCellHdlrReq */


/***************************************************************************
 * Function Name: DoUnregCellHdlrReq
 * Description  : Processes an XTMRT_CMD_UNREGISTER_CELL_HANDLER command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnregCellHdlrReq( PXTMRT_CELL_HDLR pCh )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    switch( pCh->ulCellHandlerType )
    {
    case CELL_HDLR_OAM:
        if( pGi->pfnOamHandler == pCh->pfnCellHandler )
        {
            pGi->pfnOamHandler = NULL;
            pGi->pOamContext = NULL;
        }
        else
            nRet = -EPERM;
        break;

    case CELL_HDLR_ASM:
        if( pGi->pfnAsmHandler == pCh->pfnCellHandler )
        {
            pGi->pfnAsmHandler = NULL;
            pGi->pAsmContext = NULL;
        }
        else
            nRet = -EPERM;
        break;
    }

    return( nRet );
} /* DoUnregCellHdlrReq */


/***************************************************************************
 * Function Name: DoLinkStsChangedReq
 * Description  : Processes an XTMRT_CMD_LINK_STATUS_CHANGED command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkStsChangedReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc )
{
    int nRet = -EPERM;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    local_bh_disable();

    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] == pDevCtx )
        {
            pDevCtx->ulFlags |= pLsc->ulLinkState & LSC_RAW_ENET_MODE;
            pLsc->ulLinkState &= ~LSC_RAW_ENET_MODE;
            pDevCtx->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;
            pDevCtx->MibInfo.ulIfSpeed = pLsc->ulLinkUsRate;

            if( pLsc->ulLinkState == LINK_UP )
                nRet = DoLinkUp( pDevCtx, pLsc );
            else
                nRet = DoLinkDown( pDevCtx );
            break;
        }

    local_bh_enable();

    return( nRet );
} /* DoLinkStsChangedReq */


/***************************************************************************
 * Function Name: DoLinkUp
 * Description  : Processes a "link up" condition.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkUp( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc )
{
    int nRet = 0;
    volatile DmaChannelCfg *pRxDma;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId;
    PTXQINFO pTxQInfo;
    UINT32 i;

    /* Initialize transmit DMA channel information. */
    pDevCtx->ucTxVcid = pLsc->ucTxVcid;
    pDevCtx->ulLinkState = pLsc->ulLinkState;
    pDevCtx->ulTxQInfosSize = 0;

    /* Use each Rx vcid as an index into an array of bcmxtmrt devices
     * context structures.
     */
    for( i = 0; i < pLsc->ulRxVcidsSize; i++ )
        pGi->pDevCtxsByMatchId[pLsc->ucRxVcids[i]] = pDevCtx;

    for(i = 0,pTxQInfo = pDevCtx->TxQInfos, pTxQId = pLsc->TransitQueueIds;
        i < pLsc->ulTransmitQueueIdsSize && nRet == 0; i++, pTxQInfo++, pTxQId++)
    {
        nRet = DoSetTxQueue(pDevCtx, pTxQId );
    }

    if( nRet == 0 )
    {
        /* If it is not already there, put the driver into a "ready to send and
         * receive state".
         */
        if( pGi->ulDrvState == XTMRT_INITIALIZED )
        {
            /* Enable receive interrupts and start a timer. */
            for( i = 0, pRxDma = pGi->pRxDmaBase; i < MAX_RECEIVE_QUEUES;
                i++, pRxDma++ )
            {
                if( pGi->RxBdInfos[i].pBdBase )
                {
                    pRxDma->cfg = DMA_ENABLE;
                    BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);
                }
            }

            pGi->Timer.expires = jiffies + SAR_TIMEOUT;
            add_timer(&pGi->Timer);

            if( pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
                netif_start_queue(pDevCtx->pDev);

            pGi->ulDrvState = XTMRT_RUNNING;
        }
    }
    else
    {
        /*Memory allocation error. Free memory that was previously allocated.*/
        for(i = 0, pTxQInfo = pDevCtx->TxQInfos; i < pDevCtx->ulTxQInfosSize;
            i++, pTxQInfo++ )
        {
            if( pTxQInfo->pMemBuf )
            {
                kfree(pTxQInfo->pMemBuf);
                pTxQInfo->pMemBuf = NULL;
            }
        }
        pDevCtx->ulTxQInfosSize = 0;
    }

    return( nRet );
} /* DoLinkUp */


/***************************************************************************
 * Function Name: DoLinkDown
 * Description  : Processes a "link down" condition.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkDown( PBCMXTMRT_DEV_CONTEXT pDevCtx )
{
    int nRet = 0;
    volatile DmaStateRam  *pStRam;
    volatile DmaChannelCfg *pRxDma;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PTXQINFO pTxQInfo;
    UINT32 i, j, ulIdx, ulStopRunning;

    /* Disable transmit DMA. */
    pDevCtx->ulLinkState = LINK_DOWN;
    pStRam = pGi->pDmaRegs->stram.s;
    for( i = 0, pTxQInfo = pDevCtx->TxQInfos; i < pDevCtx->ulTxQInfosSize;
         i++, pTxQInfo++ )
    {
        pTxQInfo->pDma->cfg = DMA_PKT_HALT;
        udelay(500);
        pTxQInfo->pDma->cfg = 0;
        ulIdx = SAR_TX_DMA_BASE_CHAN + pTxQInfo->ulDmaIndex;
        pStRam[ulIdx].baseDescPtr = 0;
        pStRam[ulIdx].state_data = 0;
        pStRam[ulIdx].desc_len_status = 0;
        pStRam[ulIdx].desc_base_bufptr = 0;

        /* Free transmitted packets. */
        for( j = 0; j < pTxQInfo->ulQueueSize; j++ )
            if( pTxQInfo->ppSkbs[j] && pTxQInfo->pBds[j].address )
                dev_kfree_skb_any(pTxQInfo->ppSkbs[j]);

        pTxQInfo->ulFreeBds = pTxQInfo->ulQueueSize = 0;
        pTxQInfo->ulNumTxBufsQdOne = 0;

        /* Free memory used for transmit queue. */
        if( pTxQInfo->pMemBuf )
        {
            kfree(pTxQInfo->pMemBuf);
            pTxQInfo->pMemBuf = NULL;
        }
    }

    /* Zero transmit related data structures. */
    pDevCtx->ulTxQInfosSize = 0;
    memset(pDevCtx->TxQInfos, 0x00, sizeof(pDevCtx->TxQInfos));
    memset(pDevCtx->pTxPriorities, 0x00, sizeof(pDevCtx->pTxPriorities));
    pDevCtx->ucTxVcid = INVALID_VCID;
    pGi->ulNumTxBufsQdAll = 0;

    /* Zero receive vcids. */
    for( i = 0; i < MAX_MATCH_IDS; i++ )
        if( pGi->pDevCtxsByMatchId[i] == pDevCtx )
            pGi->pDevCtxsByMatchId[i] = NULL;

    /* If all links are down, put the driver into an "initialized" state. */
    for( i = 0, ulStopRunning = 1; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] && pGi->pDevCtxs[i]->ulLinkState == LINK_UP )
        {
            ulStopRunning = 0;
            break;
        }

    if( ulStopRunning )
    {
        /* Disable receive interrupts and stop the timer. */
        for( i = 0, pRxDma = pGi->pRxDmaBase; i < MAX_RECEIVE_QUEUES;
             i++, pRxDma++ )
        {
            if( pGi->RxBdInfos[i].pBdBase )
            {
                pRxDma->cfg = 0;
                BcmHalInterruptDisable(SAR_RX_INT_ID_BASE + i);
            }
        }

        del_timer_sync(&pGi->Timer);

        pGi->ulDrvState = XTMRT_INITIALIZED;
    }

    return( nRet );
} /* DoLinkDown */


/***************************************************************************
 * Function Name: DoSetTxQueue
 * Description  : Allocate memory for and initialize a transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    int nRet = 0;
    UINT32 ulQueueSize, ulPort, ulSize;
    UINT8 *p;
    PTXQINFO pTxQInfo =  &pDevCtx->TxQInfos[pDevCtx->ulTxQInfosSize++];
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    /* Set every transmit queue size to the number of external buffers.
     * The QueuePacket function will control how many packets are queued.
     */
    ulQueueSize = pGi->ulNumExtBufs;

    ulSize = (ulQueueSize * (sizeof(struct sk_buff *) +
        sizeof(struct DmaDesc))) + 0x20; /* 0x20 for alignment */

    if( (pTxQInfo->pMemBuf = kmalloc(ulSize, GFP_ATOMIC)) != NULL )
    {
        memset(pTxQInfo->pMemBuf, 0x00, ulSize);
        cache_wbflush_len(pTxQInfo->pMemBuf, ulSize);


        ulPort = PORTID_TO_PORT(pTxQId->ulPortId);
        if( ulPort < MAX_PHY_PORTS &&
            pTxQId->ulSubPriority < MAX_SUB_PRIORITIES )
        {
            volatile DmaStateRam *pStRam = pGi->pDmaRegs->stram.s;
            UINT32 i, ulTxQs, ulPtmPriority = 0;

            p = (UINT8 *) (((UINT32) pTxQInfo->pMemBuf + 0x0f) & ~0x0f);
            pTxQInfo->ulPort = ulPort;
            pTxQInfo->ulPtmPriority = pTxQId->ulPtmPriority;
            pTxQInfo->ulSubPriority = pTxQId->ulSubPriority;
            pTxQInfo->ulQueueSize = ulQueueSize;
            pTxQInfo->ulDmaIndex = pTxQId->ulQueueIndex;
            pTxQInfo->pDma = pGi->pTxDmaBase + pTxQInfo->ulDmaIndex;
            pTxQInfo->pBds = (volatile DmaDesc *) CACHE_TO_NONCACHE(p);
            pTxQInfo->pBds[pTxQInfo->ulQueueSize - 1].status |= DMA_WRAP;
            p += ((sizeof(struct DmaDesc) * ulQueueSize)+0x0f) & ~0x0f;
            pTxQInfo->ppSkbs = (struct sk_buff **) p;
            pTxQInfo->ulNumTxBufsQdOne = 0;
            pTxQInfo->ulFreeBds = pTxQInfo->ulQueueSize;
            pTxQInfo->ulHead = pTxQInfo->ulTail = 0;

            pTxQInfo->pDma->cfg = 0;
            pTxQInfo->pDma->maxBurst = SAR_DMA_MAX_BURST_LENGTH;
            pTxQInfo->pDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
            memset((UINT8 *)&pStRam[SAR_TX_DMA_BASE_CHAN+pTxQInfo->ulDmaIndex],
                0x00, sizeof(DmaStateRam));
            pStRam[SAR_TX_DMA_BASE_CHAN + pTxQInfo->ulDmaIndex].baseDescPtr =
                (UINT32) VIRT_TO_PHY(pTxQInfo->pBds);

            if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM)
               ulPtmPriority = (pTxQInfo->ulPtmPriority == PTM_PRI_HIGH)? 1 : 0;
            pDevCtx->pTxPriorities[ulPtmPriority][ulPort][pTxQInfo->ulSubPriority] = pTxQInfo;

            /* Count the total number of transmit queues used across all device
             * interfaces.
             */
            for( i = 0, ulTxQs = 0; i < MAX_DEV_CTXS; i++ )
                if( pGi->pDevCtxs[i] )
                    ulTxQs += pGi->pDevCtxs[i]->ulTxQInfosSize;
            pGi->ulNumTxQs = ulTxQs;
        }
        else
        {
            printk(CARDNAME ": Invalid transmit queue port/priority\n");
            nRet = -EFAULT;
        }
    }
    else
        nRet = -ENOMEM;

    return( nRet );
} /* DoSetTxQueue */


/***************************************************************************
 * Function Name: DoUnsetTxQueue
 * Description  : Frees memory for a transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnsetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    int nRet = 0;
    UINT32 i, j, ulTxQs;
    PTXQINFO pTxQInfo;

    for( i = 0, pTxQInfo = pDevCtx->TxQInfos; i < pDevCtx->ulTxQInfosSize;
        i++, pTxQInfo++ )
    {
        if( pTxQId->ulQueueIndex == pTxQInfo->ulDmaIndex )
        {
            UINT32 ulPort = PORTID_TO_PORT(pTxQId->ulPortId);
            UINT32 ulIdx = SAR_TX_DMA_BASE_CHAN+pTxQInfo->ulDmaIndex;
            PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
            volatile DmaStateRam *pStRam = pGi->pDmaRegs->stram.s;
            UINT32 ulPtmPriority = 0;

            pTxQInfo->pDma->cfg = DMA_BURST_HALT;
            pStRam[ulIdx].baseDescPtr = 0;
            pStRam[ulIdx].state_data = 0;
            pStRam[ulIdx].desc_len_status = 0;
            pStRam[ulIdx].desc_base_bufptr = 0;

            if( pTxQInfo->pMemBuf )
            {
                kfree(pTxQInfo->pMemBuf);
                pTxQInfo->pMemBuf = NULL;
            }

            if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM)
               ulPtmPriority = (pTxQInfo->ulPtmPriority == PTM_PRI_HIGH)? 1 : 0;
            pDevCtx->pTxPriorities[ulPtmPriority][ulPort][pTxQInfo->ulSubPriority] = NULL;

            /* Shift remaining array elements down by one element. */
            memmove(pTxQInfo, pTxQInfo + 1, (pDevCtx->ulTxQInfosSize - i - 1) *
                sizeof(TXQINFO));
            pDevCtx->ulTxQInfosSize--;

            /* Count the total number of transmit queues used across all device
             * interfaces.
             */
            for( j = 0, ulTxQs = 0; j < MAX_DEV_CTXS; j++ )
                if( pGi->pDevCtxs[j] )
                    ulTxQs += pGi->pDevCtxs[j]->ulTxQInfosSize;
            ulTxQs = pGi->ulNumTxQs;

            break;
        }
    }

    return( nRet );
} /* DoUnsetTxQueue */


/***************************************************************************
 * Function Name: DoSendCellReq
 * Description  : Processes an XTMRT_CMD_SEND_CELL command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSendCellReq( PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC )
{
    int nRet = 0;

    if( pDevCtx->ulLinkState == LINK_UP )
    {
        struct sk_buff *skb = dev_alloc_skb(CELL_PAYLOAD_SIZE);

        if( skb )
        {
            UINT32 i;
            UINT32 ulPort = (pC->ConnAddr.ulTrafficType == TRAFFIC_TYPE_ATM)
                ? pC->ConnAddr.u.Vcc.ulPortMask
                : pC->ConnAddr.u.Flow.ulPortMask;
            UINT32 ulPtmPriority = 0;

            /* A network device instance can potentially have transmit queues
             * on different ports. Find a transmit queue for the port specified
             * in the cell structure.  The cell structure should only specify
             * one port.
             */
            for( i = 0; i < MAX_SUB_PRIORITIES; i++ )
            {
                if( pDevCtx->pTxPriorities[ulPtmPriority][ulPort][i] )
                {
                    skb->mark = i;
                    break;
                }
            }

            skb->dev = pDevCtx->pDev;
            __skb_put(skb, CELL_PAYLOAD_SIZE);
            memcpy(skb->data, pC->ucData, CELL_PAYLOAD_SIZE);

            switch( pC->ucCircuitType )
            {
            case CTYPE_OAM_F5_SEGMENT:
                skb->protocol = FSTAT_CT_OAM_F5_SEG;
                break;

            case CTYPE_OAM_F5_END_TO_END:
                skb->protocol = FSTAT_CT_OAM_F5_E2E;
                break;

            case CTYPE_OAM_F4_SEGMENT:
                skb->protocol = FSTAT_CT_OAM_F4_SEG;
                break;

            case CTYPE_OAM_F4_END_TO_END:
                skb->protocol = FSTAT_CT_OAM_F4_E2E;
                break;

            case CTYPE_ASM_P0:
                skb->protocol = FSTAT_CT_ASM_P0;
                break;

            case CTYPE_ASM_P1:
                skb->protocol = FSTAT_CT_ASM_P1;
                break;

            case CTYPE_ASM_P2:
                skb->protocol = FSTAT_CT_ASM_P2;
                break;

            case CTYPE_ASM_P3:
                skb->protocol = FSTAT_CT_ASM_P3;
                break;
            }

            skb->protocol |= SKB_PROTO_ATM_CELL;

            bcmxtmrt_xmit( skb, pDevCtx->pDev);
        }
        else
            nRet = -ENOMEM;
    }
    else
        nRet = -EPERM;

    return( nRet );
} /* DoSendCellReq */


/***************************************************************************
 * Function Name: DoDeleteDeviceReq
 * Description  : Processes an XTMRT_CMD_DELETE_DEVICE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoDeleteDeviceReq( PBCMXTMRT_DEV_CONTEXT pDevCtx )
{
    int nRet = -EPERM;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] == pDevCtx )
        {
            pGi->pDevCtxs[i] = NULL;

            kerSysReleaseMacAddress( pDevCtx->pDev->dev_addr );

            unregister_netdev( pDevCtx->pDev );
            free_netdev( pDevCtx->pDev );

            nRet = 0;
            break;
        }

    for( i = 0; i < MAX_MATCH_IDS; i++ )
        if( pGi->pDevCtxsByMatchId[i] == pDevCtx )
            pGi->pDevCtxsByMatchId[i] = NULL;

    return( nRet );
} /* DoDeleteDeviceReq */


/***************************************************************************
 * Function Name: DoGetNetDevTxChannel
 * Description  : Processes an XTMRT_CMD_GET_NETDEV_TXCHANNEL command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoGetNetDevTxChannel( PXTMRT_NETDEV_TXCHANNEL pParm )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    PTXQINFO pTqi;
    UINT32 i, j;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        pDevCtx = pGi->pDevCtxs[i];
        if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
        {
            if ( pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
            {
                for ( j=0, pTqi=pDevCtx->TxQInfos;
                      j<pDevCtx->ulTxQInfosSize; j++, pTqi++)
                {
                    if ( pTqi->ulDmaIndex == pParm->txChannel )
                    {
                        pParm->pDev = (void*)pDevCtx->pDev;
                        return nRet;
                    }
                }
            }
        }
    }

    return -EEXIST;
} /* DoGetNetDevTxChannel */


/***************************************************************************
 * Function Name: bcmxtmrt_add_proc_files
 * Description  : Adds proc file system directories and entries.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_add_proc_files( void )
{
    proc_mkdir ("driver/xtm", NULL);
    create_proc_read_entry("driver/xtm/txdmainfo", 0, NULL, ProcDmaTxInfo, 0);

    return(0);
} /* bcmxtmrt_add_proc_files */


/***************************************************************************
 * Function Name: bcmxtmrt_del_proc_files
 * Description  : Deletes proc file system directories and entries.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_del_proc_files( void )
{
    remove_proc_entry("driver/xtm/txdmainfo", NULL);
    remove_proc_entry("driver/xtm", NULL);

    return(0);
} /* bcmxtmrt_del_proc_files */


/***************************************************************************
 * Function Name: ProcDmaTxInfo
 * Description  : Displays information about transmit DMA channels for all
 *                network interfaces.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt, 
    int *eof, void *data)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    PTXQINFO pTqi;
    UINT32 i, j;
    int sz = 0;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        pDevCtx = pGi->pDevCtxs[i];
        if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
        {
            for( j = 0, pTqi = pDevCtx->TxQInfos;
                j < pDevCtx->ulTxQInfosSize; j++, pTqi++ )
            {
                sz += sprintf(page + sz, "dev: %s, tx_chan_size: %lu, tx_chan"
                    "_filled: %lu\n", pDevCtx->pDev->name, pTqi->ulQueueSize,
                    pTqi->ulNumTxBufsQdOne);
            }
        }
    }

    sz += sprintf(page + sz, "\next_buf_size: %lu, reserve_buf_size: %lu, tx_"
        "total_filled: %lu\n\n", pGi->ulNumExtBufs, pGi->ulNumExtBufsRsrvd,
        pGi->ulNumTxBufsQdAll);

    sz += sprintf(page + sz, "queue_condition: %lu %lu %lu, drop_condition: "
        "%lu %lu %lu\n\n", pGi->ulDbgQ1, pGi->ulDbgQ2, pGi->ulDbgQ3,
        pGi->ulDbgD1, pGi->ulDbgD2, pGi->ulDbgD3);

    *eof = 1;
    return( sz );
} /* ProcDmaTxInfo */

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init(bcmxtmrt_init);
module_exit(bcmxtmrt_cleanup);
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(bcmxtmrt_request);

