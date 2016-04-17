/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 

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
/*
 * Interrupt control functions for Broadcom 963xx MIPS boards
 */

#include <asm/atomic.h>

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/linkage.h>

#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/signal.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>


static void irq_dispatch_int(void)
{
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    uint64 pendingIrqs;
    static uint64 irqBit;
#else
    uint32 pendingIrqs;
    static uint32 irqBit;
#endif

    static uint32 isrNumber = (sizeof(irqBit) * 8) - 1;

#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    pendingIrqs = PERF->IrqStatus_high & PERF->IrqMask_high;
    pendingIrqs = (pendingIrqs << 32) | (PERF->IrqStatus & PERF->IrqMask);
#else
    pendingIrqs = PERF->IrqStatus & PERF->IrqMask;
#endif

    if (!pendingIrqs) {
        return;
    }

    while (1) {
        irqBit <<= 1;
        isrNumber++;
        if (isrNumber == (sizeof(irqBit) * 8)) {
            isrNumber = 0;
            irqBit = 0x1;
        }
        if (pendingIrqs & irqBit) {
            unsigned int irq = isrNumber + INTERNAL_ISR_TABLE_OFFSET;
#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
            if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) {
                PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
            }
            else if (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_5) {
                PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_CLEAR_SHFT));      // Clear
            }
#endif
            do_IRQ(irq);
            break;
        }
    }
}


#if defined(CONFIG_BCM96338) || defined(CONFIG_BCM96348) 
static void irq_dispatch_ext(uint32 irq)
{
    if (!(PERF->ExtIrqCfg & (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT)))) {
        printk("**** Ext IRQ mask. Should not dispatch ****\n");
    }
    /* clear interrupt in the controller */
    PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));
    do_IRQ(irq);
}
#endif


static void irq_dispatch_sw(uint32 irq)
{
    clear_c0_cause(0x1 << (CAUSEB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    do_IRQ(irq);
}


asmlinkage void plat_irq_dispatch(void)
{
    u32 cause;

    while((cause = (read_c0_cause() & read_c0_status() & CAUSEF_IP))) {
        if (cause & CAUSEF_IP7)
        {
            do_IRQ(MIPS_TIMER_INT);
        }
        else if (cause & CAUSEF_IP2)
            irq_dispatch_int();
#if defined(CONFIG_BCM96338) || defined(CONFIG_BCM96348) 
        else if (cause & CAUSEF_IP3)
            irq_dispatch_ext(INTERRUPT_ID_EXTERNAL_0);
        else if (cause & CAUSEF_IP4)
            irq_dispatch_ext(INTERRUPT_ID_EXTERNAL_1);
        else if (cause & CAUSEF_IP5)
            irq_dispatch_ext(INTERRUPT_ID_EXTERNAL_2);
        else if (cause & CAUSEF_IP6)
            irq_dispatch_ext(INTERRUPT_ID_EXTERNAL_3);
#endif
        else if (cause & CAUSEF_IP0)
            irq_dispatch_sw(INTERRUPT_ID_SOFTWARE_0);
        else if (cause & CAUSEF_IP1)
            irq_dispatch_sw(INTERRUPT_ID_SOFTWARE_1);
    }
}


void enable_brcm_irq(unsigned int irq)
{
    unsigned long flags;

    local_irq_save(flags);

    if( irq >= INTERNAL_ISR_TABLE_OFFSET ) {
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
        if (irq < INTERNAL_HIGH_ISR_TABLE_OFFSET)
            PERF->IrqMask |= (1 << (irq - INTERNAL_ISR_TABLE_OFFSET));
        else
            PERF->IrqMask_high |= (1 << (irq - INTERNAL_HIGH_ISR_TABLE_OFFSET));
#else
        PERF->IrqMask |= (1 << (irq - INTERNAL_ISR_TABLE_OFFSET));
#endif
    }
    else if ((irq == INTERRUPT_ID_SOFTWARE_0) || (irq == INTERRUPT_ID_SOFTWARE_1)) {
        set_c0_status(0x1 << (STATUSB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    }

    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) {
        PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_INSENS_SHFT));    // Edge insesnsitive
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_LEVEL_SHFT));      // Level triggered
        PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_SENSE_SHFT));     // Low level
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT));       // Unmask
    }
#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    else if (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_5) {
        PERF->ExtIrqCfg1 &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_INSENS_SHFT));    // Edge insesnsitive
        PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_LEVEL_SHFT));      // Level triggered
        PERF->ExtIrqCfg1 &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_SENSE_SHFT));     // Low level
        PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_CLEAR_SHFT));      // Clear
        PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_MASK_SHFT));       // Unmask
    }
#endif

    local_irq_restore(flags);
}


void disable_brcm_irq(unsigned int irq)
{
    unsigned long flags;

    local_irq_save(flags);
    if( irq >= INTERNAL_ISR_TABLE_OFFSET ) {
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
        if (irq < INTERNAL_HIGH_ISR_TABLE_OFFSET)
            PERF->IrqMask &= ~(1 << (irq - INTERNAL_ISR_TABLE_OFFSET));
        else
            PERF->IrqMask_high &= ~(1 << (irq - INTERNAL_HIGH_ISR_TABLE_OFFSET));
#else
        PERF->IrqMask &= ~(1 << (irq - INTERNAL_ISR_TABLE_OFFSET));
#endif
    }
    else if ((irq == INTERRUPT_ID_SOFTWARE_0) || (irq == INTERRUPT_ID_SOFTWARE_1)) {
        clear_c0_status(0x1 << (STATUSB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    }
#if defined(CONFIG_BCM96338) || defined(CONFIG_BCM96348) 
    else if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) {
        /* disable interrupt in the controller */
        PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT));
    }
#endif
    local_irq_restore(flags);
}


void unmask_brcm_irq_noop(unsigned int irq)
{
}


#if defined (CONFIG_BCM96358) || defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
#define ALLINTS_NOTIMER IE_IRQ0
#else
#define ALLINTS_NOTIMER (IE_IRQ0 | IE_IRQ1 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4)
#endif

static struct irq_chip brcm_irq_chip = {
    .name = "BCM63xx",
    .enable = enable_brcm_irq,
    .disable = disable_brcm_irq,
    .ack = disable_brcm_irq,
    .mask = disable_brcm_irq,
    .mask_ack = disable_brcm_irq,
    .unmask = enable_brcm_irq
};

static struct irq_chip brcm_irq_chip_no_unmask = {
    .name = "BCM63xx",
    .enable = enable_brcm_irq,
    .disable = disable_brcm_irq,
    .ack = disable_brcm_irq,
    .mask = disable_brcm_irq,
    .mask_ack = disable_brcm_irq,
    .unmask = unmask_brcm_irq_noop
};


void __init arch_init_irq(void)
{
    int i;

	for (i = 0; i < NR_IRQS; i++) {
		set_irq_chip_and_handler(i, &brcm_irq_chip, handle_level_irq);
	}

	clear_c0_status(ST0_BEV);
	change_c0_status(ST0_IM, ALLINTS_NOTIMER);

#ifdef CONFIG_REMOTE_DEBUG
	rs_kgdb_hook(0);
#endif
}


// This is a wrapper to standand Linux request_irq
// Differences are:
//    - The irq won't be renabled after ISR is done and needs to be explicity re-enabled, which is good for NAPI drivers.
//      The change is implemented by filling in an no-op unmask function in brcm_irq_chip_no_unmask and set it as the irq_chip
//    - IRQ flags and interrupt names are automatically set
// Either request_irq or BcmHalMapInterrupt can be used. Just make sure re-enabling IRQ is handled correctly.

unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, unsigned int param, unsigned int irq)
{
    char *devname;
    unsigned long irqflags;

    devname = kmalloc(16, GFP_KERNEL);
    if (!devname) {
        return -1;
    }
    sprintf( devname, "brcm_%d", irq );

    set_irq_chip_and_handler(irq, &brcm_irq_chip_no_unmask, handle_level_irq);

    irqflags = IRQF_DISABLED | IRQF_SAMPLE_RANDOM;

#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358) ||  defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    if( irq == INTERRUPT_ID_MPI ) {
        irqflags |= IRQF_SHARED;
    }
#endif

    return request_irq(irq, pfunc, irqflags, devname, (void *) param);
}


//***************************************************************************
//  void  BcmHalGenerateSoftInterrupt
//
//   Triggers a software interrupt.
//
//***************************************************************************
void BcmHalGenerateSoftInterrupt( unsigned int irq )
{
    unsigned long flags;

    local_irq_save(flags);

    set_c0_cause(0x1 << (CAUSEB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));

    local_irq_restore(flags);
}


EXPORT_SYMBOL(enable_brcm_irq);
EXPORT_SYMBOL(disable_brcm_irq);
EXPORT_SYMBOL(BcmHalMapInterrupt);
EXPORT_SYMBOL(BcmHalGenerateSoftInterrupt);
