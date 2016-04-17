/*
<:copyright-gpl 
 Copyright 2004 Broadcom Corp. All Rights Reserved. 
 
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
 * prom.c: PROM library initialization code.
 *
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <dsp_mod_size.h>

extern int  do_syslog(int, char *, int);
extern NVRAM_DATA bootNvramData;

void __init create_root_nfs_cmdline( char *cmdline );
UINT32 __init calculateCpuSpeed(void);


const char *get_system_type(void)
{
    return( bootNvramData.szBoardId );
}


/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */
void __init prom_init(void)
{
    extern ulong r4k_interval;

    kerSysEarlyFlashInit();

#if defined(CONFIG_PRINTK)
    do_syslog(8, NULL, 8);
#endif

    printk( "%s prom init\n", get_system_type() );

    PERF->IrqMask = 0;

    arcs_cmdline[0] = '\0';

#if defined(CONFIG_ROOT_NFS)
    create_root_nfs_cmdline( arcs_cmdline );
#elif defined(CONFIG_ROOT_FLASHFS)
    strcpy(arcs_cmdline, CONFIG_ROOT_FLASHFS);
#endif
    strcat(arcs_cmdline, " ");
    strcat(arcs_cmdline, CONFIG_CMDLINE);

    /* Count register increments every other clock */
    mips_hpt_frequency = calculateCpuSpeed() / 2;
#if 0	
    r4k_interval = mips_hpt_frequency / HZ;
#endif
    mips_machgroup = MACH_GROUP_BRCM;
    mips_machtype = MACH_BCM963XX;
}


/* --------------------------------------------------------------------------
    Name: prom_free_prom_memory
Abstract: 
 -------------------------------------------------------------------------- */
void __init prom_free_prom_memory(void)
{

}


#if defined(CONFIG_ROOT_NFS)
/* This function reads in a line that looks something like this:
 *
 *
 * CFE bootline=bcmEnet(0,0)host:vmlinux e=192.169.0.100:ffffff00 h=192.169.0.1
 *
 *
 * and retuns in the cmdline parameter some that looks like this:
 *
 * CONFIG_CMDLINE="root=/dev/nfs nfsroot=192.168.0.1:/opt/targets/96345R/fs
 * ip=192.168.0.100:192.168.0.1::255.255.255.0::eth0:off rw"
 */
#define BOOT_LINE_ADDR   0x0
#define HEXDIGIT(d) ((d >= '0' && d <= '9') ? (d - '0') : ((d | 0x20) - 'W'))
#define HEXBYTE(b)  (HEXDIGIT((b)[0]) << 4) + HEXDIGIT((b)[1])

void __init create_root_nfs_cmdline( char *cmdline )
{
    char root_nfs_cl[] = "root=/dev/nfs nfsroot=%s:" CONFIG_ROOT_NFS_DIR
        " ip=%s:%s::%s::eth0:off rw";

    char *localip = NULL;
    char *hostip = NULL;
    char mask[16] = "";
    char bootline[128] = "";
    char *p = bootline;

    memcpy(bootline, bootNvramData.szBootline, sizeof(bootline));

    while( *p )
    {
        if( p[0] == 'e' && p[1] == '=' )
        {
            /* Found local ip address */
            p += 2;
            localip = p;
            while( *p && *p != ' ' && *p != ':' )
                p++;
            if( *p == ':' )
            {
                /* Found network mask (eg FFFFFF00 */
                *p++ = '\0';
                sprintf( mask, "%u.%u.%u.%u", HEXBYTE(p), HEXBYTE(p + 2),
                HEXBYTE(p + 4), HEXBYTE(p + 6) );
                p += 4;
            }
            else if( *p == ' ' )
                *p++ = '\0';
        }
        else if( p[0] == 'h' && p[1] == '=' )
        {
            /* Found host ip address */
            p += 2;
            hostip = p;
            while( *p && *p != ' ' )
                p++;
            if( *p == ' ' )
                    *p++ = '\0';
        }
        else 
            p++;
    }

    if( localip && hostip ) 
        sprintf( cmdline, root_nfs_cl, hostip, localip, hostip, mask );
}
#endif

/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM63xx CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fcpu_clk = (25 * MIPSDDR_NDIV) / MIPS_MDIV
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
#if defined(CONFIG_BCM96338)
UINT32 __init calculateCpuSpeed(void)
{
    return 240000000;
}
#endif

#if defined(CONFIG_BCM96348)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllStrap = PERF->PllStrap;
    
    numerator = 64000000 / 4 *
        (((pllStrap & PLL_N1_MASK) >> PLL_N1_SHFT) + 1) *
        (((pllStrap & PLL_N2_MASK) >> PLL_N2_SHFT) + 2);

    cpu_speed = (numerator / (((pllStrap & PLL_M1_CPU_MASK) >> PLL_M1_CPU_SHFT) + 1));

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96358)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;

    numerator = 64000000 / 4 *
        ((pllConfig & MIPSDDR_N2_MASK) >> MIPSDDR_N2_SHFT) * 
        ((pllConfig & MIPSDDR_N1_MASK) >> MIPSDDR_N1_SHFT);

    cpu_speed = numerator / ((pllConfig & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96368)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;
    UINT32 pllMDiv = DDR->MIPSDDRPLLMDiv;

    numerator = 64000000 / ((pllConfig & MIPSDDR_P1_MASK) >> MIPSDDR_P1_SHFT) * 
        ((pllConfig & MIPSDDR_P2_MASK) >> MIPSDDR_P2_SHFT) *
        ((pllConfig & MIPSDDR_NDIV_MASK) >> MIPSDDR_NDIV_SHFT);

    cpu_speed = numerator / ((pllMDiv & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96816)
UINT32 __init calculateCpuSpeed(void)
{
    return 400000000;
}
#endif

