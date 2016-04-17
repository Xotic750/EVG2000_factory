/*
 * DSLCPE specific functions
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <typedefs.h>
#include <linuxver.h>

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>

#include <osl.h>
#if !defined(LINUX_HYBRID) && !defined(WL_UMK)
#include <wlc_cfg.h>
#endif

/* #include <wl_dbg.h> */
#include <wlioctl.h>
#include <wl_linux_dslcpe.h>

typedef const struct si_pub	si_t;
#include <bcmutils.h>
#include <wlc_pub.h>

#include <wl_linux.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#ifdef DSLCPE_SDIO
#include <bcmsdh.h>
extern wl_info_t *sdio_dev;
void *bcmsdh_osh = NULL;
#endif /* DSLCPE_SDIO */

/* USBAP */
#ifdef BCMDBUS
#include "dbus.h"
/* BMAC_NOTES: Remove, but just in case your Linux system has this defined */
#undef CONFIG_PCI
void *wl_dbus_probe_cb(void *arg, const char *desc, uint32 bustype, uint32 hdrlen);
void wl_dbus_disconnect_cb(void *arg);
#endif
#include "bcm_map.h"
#include "bcm_intr.h"
#include "board.h"
#include "bcmnet.h"
#include "boardparms.h"

#ifdef BCMDBG
extern int msglevel;
#endif
extern struct pci_device_id wl_id_table[];

extern bool
wlc_chipmatch(uint16 vendor, uint16 device);

/*
 * wl_dslcpe_open:
 * extended hook for device open for DSLCPE.
 */
int wl_dslcpe_open(struct net_device *dev)
{
	return 0;
}

/*
 * wl_dslcpe_close:
 * extended hook for device close for DSLCPE.
 */
int wl_dslcpe_close(struct net_device *dev)
{
	return 0;
}
/*
 * wlc_dslcpe_boardflags:
 * extended hook for modifying boardflags for DSLCPE.
 */
void wlc_dslcpe_boardflags(uint32 *boardflags, uint32 *boardflags2)
{
	return;
}

/*
 * wlc_dslcpe_led_attach:
 * extended hook for when led is to be initialized for DSLCPE.
 */
void wlc_dslcpe_led_attach(void)
{
	return;
}

/*
 * wlc_dslcpe_led_detach:
 * extended hook for when led is to be de-initialized for DSLCPE.
 */
void wlc_dslcpe_led_detach(void)
{
	return;
}
/*
 * wlc_dslcpe_timer_led_blink_timer:
 * extended hook for when periodical(10ms) led timer is called for DSLCPE when wlc is up.
 */
void wlc_dslcpe_timer_led_blink_timer(void)
{
	return;
}
/*
 * wlc_dslcpe_led_timer:
 * extended hook for when led blink timer(200ms) is called for DSLCPE when wlc is up.
 */
void wlc_dslcpe_led_timer(void)
{
	return;
}

/*
 * wl_dslcpe_ioctl:
 * extended ioctl support on BCM63XX.
 */
int
wl_dslcpe_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int isup = 0;
	int error = -1;

	if (cmd >= SIOCGLINKSTATE && cmd < SIOCLAST) {
		error = 0;
		/* we can add sub-command in ifr_data if we need to in the future */
		switch (cmd) {
			case SIOCGLINKSTATE:
				if (dev->flags&IFF_UP) isup = 1;
				if (copy_to_user((void*)(int*)ifr->ifr_data, (void*)&isup,
					sizeof(int))) {
					return -EFAULT;
				}
				break;
			case SIOCSCLEARMIBCNTR:
				wl_reset_cnt(dev);
				break;
		}
	} else {
		error = wl_ioctl(dev, ifr, cmd);
	}
	return error;
}

#if defined(DSLCPE_SDIO) || defined(CONFIG_PCI)
/*
 * wl_dslcpe_isr:
 */
irqreturn_t
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
wl_dslcpe_isr(int irq, void *dev_id)
#else
wl_dslcpe_isr(int irq, void *dev_id, struct pt_regs *ptregs)
#endif	/* < 2.6.20 */
{
	bool ours;

#ifdef DSLCPE_SDIO
	ours = bcmsdh_intr_handler(wl_sdh_get((wl_info_t *)dev_id));
#else
	/* ignore wl_isr return value due to dedicated interrupt line */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	ours = wl_isr(irq, dev_id);
#else
	ours = wl_isr(irq, dev_id, ptregs);
#endif	/* < 2.6.20 */
#endif /* DSLCPE_SDIO */

	BcmHalInterruptEnable(irq);

	return IRQ_RETVAL(ours);
}

/* special deal for dslcpe */
int __devinit
wl_dslcpe_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	wl_info_t *wl;
	struct net_device *dev;
	int irq;

#ifdef DSLCPE_SDIO
	wl = (wl_info_t*)sdio_dev;
#else
	if (wl_pci_probe(pdev, ent))
		return -ENODEV;

	wl = pci_get_drvdata(pdev);
#endif
	ASSERT(wl);

	/* hook ioctl */
	dev = wl_netdev_get(wl);
	ASSERT(dev);
	ASSERT(dev->do_ioctl);
	dev->do_ioctl = wl_dslcpe_ioctl;

#ifdef DSLCPE_DGASP
	kerSysRegisterDyingGaspHandler(dev->name, &wl_shutdown_handler, wl);
#endif

#ifdef DSLCPE_SDIO
	irq = dev->irq;
#else
	irq = pdev->irq;
#endif
	if (irq && wl) {
		free_irq(irq, wl);
		if (BcmHalMapInterrupt((FN_ISR)wl_dslcpe_isr, (unsigned int)wl, irq)) {
			printk("wl_dslcpe: request_irq() failed\n");
			goto fail;
		}
		BcmHalInterruptEnable(irq);
	}

	return 0;

fail:
	wl_free(wl);
	return -ENODEV;
}

#ifndef DSLCPE_SDIO
void __devexit wl_remove(struct pci_dev *pdev);

static struct pci_driver wl_pci_driver = {
	name:		"wl",
	probe:		wl_dslcpe_probe,
	remove:		__devexit_p(wl_remove),
	id_table:	wl_id_table,
	};
#endif
#endif  /* defined(DSLCPE_SDIO) || defined(CONFIG_PCI) */

/* USBAP  Could combined with wl_dslcpe_probe */
#ifdef BCMDBUS
static void *wl_dslcpe_dbus_probe_cb(void *arg, const char *desc, uint32 bustype, uint32 hdrlen)
{
	struct net_device *dev;
	wl_info_t *wl = wl_dbus_probe_cb(arg, desc, bustype, hdrlen);
	int irq;

	ASSERT(wl);

	/* hook ioctl */
	dev = wl_netdev_get(wl);
	ASSERT(dev);
	ASSERT(dev->do_ioctl);
	dev->do_ioctl = wl_dslcpe_ioctl;

#ifdef DSLCPE_DGASP
	kerSysRegisterDyingGaspHandler(dev->name, &wl_shutdown_handler, wl);
#endif
	return 0;
}

static void wl_dslcpe_dbus_disconnect_cb(void *arg)
{
	wl_dbus_disconnect_cb(arg);
}
#endif /* BCMDBUS */

static int __init
wl_module_init(void)
{

	int error;

#ifdef BCMDBG
	if (msglevel != 0xdeadbeef) {
		/* wl_msg_level = msglevel; */
		printf("%s: msglevel set to 0x%x\n", __FUNCTION__, msglevel);
	}
#endif /* BCMDBG */

#ifdef DSLCPE_SDIO
	bcmsdh_osh = osl_attach(NULL, SDIO_BUS, FALSE);
	
	if (!(error = wl_sdio_register(VENDOR_BROADCOM, BCM4318_CHIP_ID, (void *)0xfffe2300, bcmsdh_osh, INTERRUPT_ID_SDIO))) {
		if((error = wl_dslcpe_probe(0, 0)) != 0) {	/* to hookup entry points or misc */
			osl_detach(bcmsdh_osh);
			return error;
		}
	} else {
		osl_detach(bcmsdh_osh);
	}

#endif  /* DSLCPE_SDIO */

#ifdef CONFIG_PCI
	if (!(error = pci_module_init(&wl_pci_driver)))
		return (0);
#endif /* CONFIG_PCI */

#ifdef BCMDBUS
	/* BMAC_NOTE: define hardcode number, why NODEVICE is ok ? */
	error = dbus_register(BCM_DNGL_VID, BCM_DNGL_BDC_PID, wl_dslcpe_dbus_probe_cb,
		wl_dslcpe_dbus_disconnect_cb, NULL, NULL, NULL);
	if (error == DBUS_ERR_NODEVICE) {
		error = DBUS_OK;
	}
#endif /* BCMDBUS */
	return (error);
}

static void __exit
wl_module_exit(void)
{
#ifdef DSLCPE_SDIO
	wl_sdio_unregister();
	osl_detach(bcmsdh_osh);
#endif /* DSLCPE_SDIO */

#ifdef CONFIG_PCI
	pci_unregister_driver(&wl_pci_driver);
#endif	/* CONFIG_PCI */

#ifdef BCMDBUS
	dbus_deregister();
#endif /* BCMDBUS */
}

/* Turn 63xx GPIO LED On(1) or Off(0) */
void wl_dslcpe_led(unsigned char state)
{
/*if WLAN LED is from 63XX GPIO Line, define compiler flag GPIO_LED_FROM_63XX
#define GPIO_LED_FROM_63XX
*/

#ifdef GPIO_LED_FROM_63XX
	BOARD_LED_STATE led;
	led = state? kLedStateOn : kLedStateOff;

	kerSysLedCtrl(kLedSes, led);
#endif
}

module_init(wl_module_init);
module_exit(wl_module_exit);
MODULE_LICENSE("Proprietary");
