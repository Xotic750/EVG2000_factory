/*
 * wl_linux.c exported functions and definitions
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wl_linux.h,v 1.22.2.5 2009/06/19 00:26:13 Exp $
 */

#ifndef _wl_linux_h_
#define _wl_linux_h_


/* BMAC Note: High-only driver is no longer working in softirq context as it needs to block and
 * sleep so perimeter lock has to be a semaphore instead of spinlock. This requires timers to be
 * submitted to workqueue instead of being on kernel timer
 */
typedef struct wl_timer {
	struct timer_list timer;
	struct wl_info *wl;
	void (*fn)(void *);
	void* arg; /* argument to fn */
	uint ms;
	bool periodic;
	bool set;
	struct wl_timer *next;
#ifdef BCMDBG
	char* name; /* Description of the timer */
#endif
} wl_timer_t;

/* contortion to call functions at safe time */
/* In 2.6.20 kernels work functions get passed a pointer to the struct work, so things
 * will continue to work as long as the work structure is the first component of the task structure.
 */
typedef struct wl_task {
	struct work_struct work;
	void *context;
} wl_task_t;

#define WL_IFTYPE_BSS	1 /* iftype subunit for BSS */
#define WL_IFTYPE_WDS	2 /* iftype subunit for WDS */
#define WL_IFTYPE_MON	3 /* iftype subunit for MONITOR */

#ifndef WL_UMK
typedef struct wl_if {
#else
struct wl_if {
#endif
#ifdef CONFIG_WIRELESS_EXT
	wl_iw_t		iw;		/* wireless extensions state (must be first) */
#endif /* CONFIG_WIRELESS_EXT */
	struct wl_if *next;
	struct wl_info *wl;		/* back pointer to main wl_info_t */
	struct net_device *dev;		/* virtual netdevice */
	int type;			/* interface type: WDS, BSS */
	struct wlc_if *wlcif;		/* wlc interface handle */
	struct ether_addr remote;	/* remote WDS partner */
	uint subunit;			/* WDS/BSS unit */
	bool dev_registed;		/* netdev registed done */
#ifdef WL_UMK
	struct pci_dev *pci_dev;
#endif
#ifndef WL_UMK
} wl_if_t;
#else
};
#endif

struct wl_info {
	wlc_pub_t	*pub;		/* pointer to public wlc state */
	void		*wlc;		/* pointer to private common os-independent data */
	osl_t		*osh;		/* pointer to os handler */
	struct net_device *dev;		/* backpoint to device */
#ifdef WL_UMK
	wl_cdev_t	*char_dev;	/* allcoated in wl_cdev_init */
	int		pid;		/* process id */
#endif
#ifdef WLC_HIGH_ONLY
	struct semaphore sem;		/* use semaphore to allow sleep */
#else
	spinlock_t	lock;		/* per-device perimeter lock */
	spinlock_t	isr_lock;	/* per-device ISR synchronization lock */
#endif
	uint		bustype;	/* bus type */
	bool		piomode;	/* set from insmod argument */
	void *regsva;			/* opaque chip registers virtual address */
	struct net_device_stats stats;	/* stat counter reporting structure */
	wl_if_t *if_list;		/* list of all interfaces */
	struct wl_info *next;		/* pointer to next wl_info_t in chain */
	atomic_t callbacks;		/* # outstanding callback functions */
	struct wl_timer *timers;	/* timer cleanup queue */
	struct tasklet_struct tasklet;	/* dpc tasklet */
	struct net_device *monitor;	/* monitor pseudo device */
#ifdef DSLCPE_SDIO
	bcmsdh_info_t	*sdh;		/* pointer to sdio bus handler */
	osl_t		*bcmsdh_osh;	/* pointer to os handler */
	ulong		flags;		/* current irq flags */
#endif /* DSLCPE_SDIO */
	bool		resched;	/* dpc needs to be and is rescheduled */
#ifdef TOE
	wl_toe_info_t	*toei;		/* pointer to toe specific information */
#endif
#ifdef ARPOE
	wl_arp_info_t	*arpi;		/* pointer to arp agent offload info */
#endif
#if defined(DSLCPE) && defined(DSLCPE_DELAY)
	shared_osl_t	oshsh;		/* shared info for osh */
#endif /* DSLCPE && DSLCPE_DELAY */
#ifdef LINUXSTA_PS
	uint32		pci_psstate[16];	/* pci ps-state save/restore */
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	struct lib80211_crypto_ops *tkipmodops;
#else
	struct ieee80211_crypto_ops *tkipmodops;	/* external tkip module ops */
#endif
	struct ieee80211_tkip_data  *tkip_ucast_data;
	struct ieee80211_tkip_data  *tkip_bcast_data;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
	/* RPC, handle, lock, txq, workitem */
#ifdef WLC_HIGH_ONLY
	rpc_info_t 	*rpc;		/* RPC handle */
	rpc_tp_info_t	*rpc_th;	/* RPC transport handle */
	wlc_rpc_ctx_t	rpc_dispatch_ctx;

	bool	   rpcq_dispatched;	/* Avoid scheduling multiple tasks */
	spinlock_t rpcq_lock;		/* Lock for the queue */
	rpc_buf_t *rpcq_head;		/* RPC Q */
	rpc_buf_t *rpcq_tail;		/* Points to the last buf */

	bool	   txq_dispatched;	/* Avoid scheduling multiple tasks */
	spinlock_t txq_lock;		/* Lock for the queue */
	struct sk_buff *txq_head;	/* TX Q */
	struct sk_buff *txq_tail;	/* Points to the last buf */

	wl_task_t	txq_task;	/* work queue for wl_start() */
	wl_task_t	multicast_task;	/* work queue for wl_set_multicast_list() */
#endif /* WLC_HIGH_ONLY */
	uint	stats_id;		/* the current set of stats */
	/* ping-pong stats counters updated by Linux watchdog */
	struct net_device_stats stats_watchdog[2];
#ifdef CONFIG_WIRELESS_EXT
	struct iw_statistics wstats_watchdog[2];
	struct iw_statistics wstats;
	int		phy_noise;
#endif /* CONFIG_WIRELESS_EXT */

};


#ifdef WLC_HIGH_ONLY
#define WL_LOCK(wl)	down(&(wl)->sem)
#define WL_UNLOCK(wl)	up(&(wl)->sem)

#define WL_ISRLOCK(wl)
#define WL_ISRUNLOCK(wl)
#else
/* perimeter lock */
#ifndef DSLCPE_SDIO
#define WL_LOCK(wl)	spin_lock_bh(&(wl)->lock)
#define WL_UNLOCK(wl)	spin_unlock_bh(&(wl)->lock)

/* locking from inside wl_isr */
#define WL_ISRLOCK(wl, flags) do {spin_lock(&(wl)->isr_lock); (void)(flags);} while (0)
#define WL_ISRUNLOCK(wl, flags) do {spin_unlock(&(wl)->isr_lock); (void)(flags);} while (0)

/* locking under WL_LOCK() to synchronize with wl_isr */
#define INT_LOCK(wl, flags)	spin_lock_irqsave(&(wl)->isr_lock, flags)
#define INT_UNLOCK(wl, flags)	spin_unlock_irqrestore(&(wl)->isr_lock, flags)
#else /* DSLCPE_SDIO */
#define WL_LOCK(wl)	\
	do { ulong flags; \
		spin_lock_irqsave(&(wl)->lock, flags); \
		(wl)->flags = flags; \
	} while (0)
#define WL_UNLOCK(wl)	\
	do { ulong flags; \
		flags = (wl)->flags; \
		spin_unlock_irqrestore(&(wl)->lock, flags); \
	} while (0)

/* locking from inside wl_isr */
#define WL_ISRLOCK(wl, flags) \
	do {spin_lock(&(wl)->lock); (void)(flags);} while (0)
#define WL_ISRUNLOCK(wl, flags) \
	do {spin_unlock(&(wl)->lock); (void)(flags);} while (0)

/* locking under WL_LOCK() to synchronize with wl_isr */
#define INT_LOCK(wl, flags)	(void)(flags)
#define INT_UNLOCK(wl, flags)	(void)(flags)
#endif /* DSLCPE_SDIO */
#endif	/* WLC_HIGH_ONLY */


#ifdef WL_UMK
extern int wl_found;
#endif

/* handle forward declaration */
typedef struct wl_info wl_info_t;

#ifndef PCI_D0
#define PCI_D0		0
#endif

#ifndef PCI_D3hot
#define PCI_D3hot	3
#endif

/* exported functions */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
extern irqreturn_t wl_isr(int irq, void *dev_id);
#else
extern irqreturn_t wl_isr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif

extern int __devinit wl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
extern void wl_free(wl_info_t *wl);
extern int  wl_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
extern struct net_device * wl_netdev_get(wl_info_t *wl);

#ifdef BCM_WL_EMULATOR
extern wl_info_t *  wl_wlcreate(osl_t *osh, void *pdev);
#endif

#ifdef DSLCPE
extern void wl_shutdown_handler(wl_info_t *wl);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11)
extern int wl_suspend(struct pci_dev *pdev, DRV_SUSPEND_STATE_TYPE  state);
#else
extern int wl_suspend(struct pci_dev *pdev, u32 state);
#endif	/* < 2.6.11 */
extern void wl_reset_cnt(struct net_device *dev);
#ifdef DSLCPE_SDIO
extern void * wl_sdh_get(wl_info_t *wl);
extern int wl_sdio_register(uint16 venid, uint16 devid, void *regsva, void *param, int irq);
extern int wl_sdio_unregister(void);
#endif /* DSLCPE_SDIO */
#endif /* DSLCPE */

#endif /* _wl_linux_h_ */
