/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
# 
# 
# This program may be linked with other software licensed under the GPL. 
# When this happens, this program may be reproduced and distributed under 
# the terms of the GPL. 
# 
# 
# 1. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH 
#    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
#    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
#    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
#    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
#    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR 
#    PERFORMANCE OF THE SOFTWARE. 
# 
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR 
#    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, 
#    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
#    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN 
#    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
#    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE 
#    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS 
#    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY 
#    LIMITED REMEDY. 
#
 * 
 ************************************************************************/

/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __OAL_H__
#define __OAL_H__

#include "cms.h"
#include "cms_eid.h"
#include "cms_log.h"
#include "cms_tms.h"
#include "cms_net.h"
#include "prctl.h"


extern void oalLog_init(void);
extern void oalLog_syslog(CmsLogLevel level, const char *buf);
extern void oalLog_cleanup(void);

/* in oal_readlog.c (legacy method of reading syslog) */
extern int oal_readLogPartial(int ptr, char* buffer);


void *oal_malloc(UINT32 size);
void oal_free(void *buf);

/* in oal_timestamp.c */
void oalTms_get(CmsTimestamp *tms);
CmsRet oalTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen);

/* in oal_strconv.c */
CmsRet oal_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val);
CmsRet oal_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val);

/* in oal_pid.c */
SINT32 oal_getPid(void);

/* in oal_prctl.c */
extern CmsRet oal_spawnProcess(const SpawnProcessInfo *spawnInfo, SpawnedProcessInfo *procInfo);
extern CmsRet oal_collectProcess(const CollectProcessInfo *collectInfo, SpawnedProcessInfo *procInfo);
extern CmsRet oal_terminateProcessGracefully(SINT32 pid);
extern CmsRet oal_terminateProcessForcefully(SINT32 pid);
extern CmsRet oal_signalProcess(SINT32 pid, SINT32 sig);

/* in oal_network.c */
extern CmsRet oal_getLanInfo(const char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask);
extern UBOOL8 oal_isInterfaceUp(const char *ifname);
#ifdef DMP_X_BROADCOM_COM_IPV6_1 /* aka SUPPORT_IPV6 */
extern CmsRet oal_getLanAddr6(const char *ifname, char *ipAddr);
extern CmsRet oal_getIfAddr6(const char *ifname, UINT32 addrIdx,
                      char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags);
#endif

/* in oal_file.c */
extern UBOOL8 oalFil_isFilePresent(const char *filename);
extern SINT32 oalFil_getSize(const char *filename);
extern CmsRet oalFil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize);

#endif /* __OAL_H__ */
