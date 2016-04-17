/**************************************************************************
#***
#***    Copyright 2008  Hon Hai Precision Ind. Co. Ltd.
#***    All Rights Reserved.
#***    No portions of this material shall be reproduced in any form without the
#***    written permission of Hon Hai Precision Ind. Co. Ltd.
#***
#***    All information contained in this document is Hon Hai Precision Ind.  
#***    Co. Ltd. company private, proprietary, and trade secret property and 
#***    are protected by international intellectual property laws and treaties.
#***
#****************************************************************************
***
***   Filename: ambitCfg.h
***
***   Description:
***         This file is specific to each project. Every project should have a
***    different copy of this file.
***        Included from ambit.h which is shared by every project.
***
***   History:
***
***   Modify Reason             Author          Date                Search Flag(Option)
***-------------------------------------------------------------------------------------
***   File Creation             Jasmine Yang    11/02/2005
*******************************************************************************/


#ifndef _AMBITCFG_H
#define _AMBITCFG_H

#define WW_VERSION           1 /* WW SKUs */
#define NA_VERSION           2 /* NA SKUs */
#define JP_VERSION           3
#define GR_VERSION           4
#define PR_VERSION           5
#define KO_VERSION           6

#define WLAN_REGION          WW_VERSION
#define FW_REGION            WW_VERSION   /* true f/w region */

/*formal version control*/
#define AMBIT_HARDWARE_VERSION     "U12H154T00"
#define AMBIT_SOFTWARE_VERSION     "V2.0.0.4"
#if (WLAN_REGION == WW_VERSION)
#define AMBIT_UI_VERSION           "2.0.4" 
#elif (WLAN_REGION == NA_VERSION)
#define AMBIT_UI_VERSION           "0.1.9NA"
#else
#error "The value of WLAN_REGION is incorrect."
#endif

#define AMBIT_PRODUCT_NAME          "EVG2000" 
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Voice Gateway EVG2000"
#define UPnP_MODEL_URL              "EVG2000.aspx"
#define UPnP_MODEL_DESCRIPTION      "RangeMax NEXT"

#define AMBIT_NVRAM_VERSION  "1" /* digital only */

#ifdef AMBIT_UPNP_SA_ENABLE /* Jasmine Add, 10/24/2006 */
#define SMART_WIZARD_SPEC_VERSION "0.7"  /* This is specification version of smartwizard 2.0 */
#endif

/*ambit, add start, william lin, 05/17/2001 */
/****************************************************************************
 ***
 ***        put AMBIT features here!!!
 ***
 ***
 ****************************************************************************/

#define ETHERNET_NAME_NUM   "eth5"
#define WLAN_IF_NAME_NUM    "wl0"       /* Mulitple BSSID #1 == Primary SSID */
#define WLAN_BSS1_NAME_NUM  "wl0.1"     /* Multiple BSSID #2 */
#define WLAN_BSS2_NAME_NUM  "wl0.2"     /* Multiple BSSID #3 */
#define WLAN_BSS3_NAME_NUM  "wl0.3"     /* Multiple BSSID #4 */

#define NUM_WAN_INTERFACE       8
#define NUM_LAN_INTERFACE       4
#define NUM_INTERFACE_GROUPS    4
#define NUM_VLAN_GROUPS         4

/*definitions: GPIOs, MTD*/
#define KERNEL_MTD_RD       "/dev/mtdblock1"
#define KERNEL_MTD_WR       "/dev/mtd1"
#define ROOTFS_MTD_RD       "/dev/mtdblock1"
#define ROOTFS_MTD_WR       "/dev/mtd1"

#define SP_MTD_RD           "/dev/mtdblock2"
#define SP_MTD_WR           "/dev/mtd2"
#define MISC_MTD_RD         "/dev/mtdblock3"
#define MISC_MTD_WR         "/dev/mtd3"
#define POT_MTD_RD          "/dev/mtdblock4"
#define POT_MTD_WR          "/dev/mtd4"
#define BD_MTD_RD           "/dev/mtdblock5"
#define BD_MTD_WR           "/dev/mtd5"
#define NVRAM_MTD_RD        "/dev/mtdblock6"
#define NVRAM_MTD_WR        "/dev/mtd6"

/*add start, from solosCommonCode, water, 10/29/2008, @cert porting*/
#define CER_DATA_OFFSET      0
/*add end, from solosCommonCode, water, 10/29/2008, @cert porting*/
#define GPIO_POWER_GREEN_LED    22
#define GPIO_POWER_RED_LED      23
#define GPIO_ETHERNET_LED       24  //Giga LAN LED
#define GPIO_WAN_LED            4   //INET_FAIL_LED
#define GPIO_WAN_LEN_CONTROL    27

#define GPIO_WPS_BUTTON         26
#define GPIO_RST_BUTTON         25

#ifdef SKN1_DECT
#define GPIO_DECT_BUTTON        26   /* foe dect paging and registration */
#define GPIO_DECT_LED           15   /* registration LED indicator */
#endif


/* wklin added start, 11/22/2006 */
/* The following definition is used in acosNvramConfig.c and acosNvramConfig.h
 * to distingiush between Fiji's and Broadcom's implementation.
 */
#define BRCM_NVRAM          /* use broadcom nvram instead of ours */

/* The following definition is to used as the key when doing des
 * encryption/decryption of backup file.
 * Have to be 7 octects.
 */
#define BACKUP_FILE_KEY         "NtgrBak"
/* wklin added end, 11/22/2006 */

#endif /*_AMBITCFG_H*/
