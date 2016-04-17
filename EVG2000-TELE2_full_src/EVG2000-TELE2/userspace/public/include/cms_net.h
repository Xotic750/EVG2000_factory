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


#ifndef __CMS_NET_H__
#define __CMS_NET_H__

/*!\file cms_net.h
 * \brief Header file for network utilities that do not require
 *  access to the MDM.
 */


#include "cms.h"
#include <arpa/inet.h>  /* mwang_todo: should not include OS dependent file */

/** Get LAN IP address and netmask.
 *
 * @param ifname (IN) name of LAN interface.
 * @param lan_ip (OUT) in_addr of the LAN interface.
 * @param lan_subnetmask (OUT) in_addr of the LAN interface subnet mask.
 *
 * @return CmsRet enum.
 */
CmsRet cmsNet_getLanInfo(const char *ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask);


/** Return TRUE if the specified interface is UP.
 *
 * @param ifname (IN) name of the interface.
 *
 * @return TRUE if the specified interface is UP.
 */
UBOOL8 cmsNet_isInterfaceUp(const char *ifname);


/** Return TRUE if the specified IP address is on the LAN side.
 *
 * @param ipAddr (IN) IP address in question.
 *
 * @return TRUE if the specified IP address is on the LAN side.
 */
UBOOL8 cmsNet_isAddressOnLanSide(const char *ipAddr);


/** Return netmask information.
 *
 * @param mask (IN) .
 */
SINT32 cmsNet_getLeftMostOneBitsInMask(char *mask);

/** Return the ifindex of an interface.
 *
 * @param ifname (IN) .
 * @return ifindex of the interface.
 */
SINT32 cmsNet_getIfindexByIfname(char *ifname);


#ifdef DMP_X_BROADCOM_COM_IPV6_1 /* aka SUPPORT_IPV6 */
CmsRet cmsNet_getIfAddr6(const char *ifname, UINT32 addrIdx,
                         char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags);
UBOOL8 cmsNet_areIp6AddrEqual(const char *ip6Addr1, const char *ip6Addr2);
UBOOL8 cmsNet_areIp6DnsEqual(const char *dnsServers1, const char *dnsServers2);
UBOOL8 cmsNet_areIp6SubnetPrefixEqual(const char *sp1, const char *sp2);
CmsRet cmsNet_subnetIp6SitePrefix(const char *sp, UINT8 subnetId, UINT32 snPlen, char *snPrefix);
#endif

/** first sub interface number for virtual ports, e.g. eth1.2, eth1.3 */
#define START_PMAP_ID           2

/** Max vendor id string len */
#define DHCP_VENDOR_ID_LEN      64

/** Maximum number of vendor id strings we can support in WebUI for portmapping. 
 * 
 * This is an arbitrary limit from the WebUI, but it propagates through to
 * utility functions dealing with DHCP Vendor Id for port mapping.
 */
#define MAX_PORTMAPPING_DHCP_VENDOR_IDS     5


#endif /* __CMS_NET_H__ */
