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

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include "cms.h"
#include "cms_util.h"
#include "oal.h"

static SINT32 cmsNet_getLeftMostOneBits(SINT32 num);

CmsRet cmsNet_getLanInfo(const char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask)
{
   return (oal_getLanInfo(lan_ifname, lan_ip, lan_subnetmask));
}


UBOOL8 cmsNet_isInterfaceUp(const char *ifname)
{
   return (oal_isInterfaceUp(ifname));
}


UBOOL8 cmsNet_isAddressOnLanSide(const char *ipAddr)
{
   UBOOL8 onLanSide = FALSE;

   /* determine the address family of ipAddr */
   if ((ipAddr != NULL) && (strchr(ipAddr, ':') == NULL))
   {
      /* ipv4 address */
      struct in_addr clntAddr, inAddr, inMask;

#ifdef DESKTOP_LINUX
      /*
       * Many desktop linux tests occur on loopback interface.  Consider that
       * as LAN side.
       */
      if (!strcmp(ipAddr, "127.0.0.1"))
      {
         return TRUE;
      }
#endif

      clntAddr.s_addr = inet_addr(ipAddr);

      oal_getLanInfo("br0", &inAddr, &inMask);
      /* check ip address of support user to see it is in LAN or not */
      if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
         onLanSide = TRUE;
      else {
         /* check ip address of support user to see if it is from secondary LAN */
         if (oal_isInterfaceUp("br0:0")) {
            oal_getLanInfo("br0:0", &inAddr, &inMask);
            if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
               onLanSide = TRUE;
         }

#ifdef mwang_todo
         /* Last option it must be from WAN side */
         /* This is something about PPP IP extension.  See bcmSetIpExtInfo. */
         if (isIpExtension()) {
            getIpExtIp(wan);
         if ( clntAddr.s_addr == inet_addr(wan) )
            onLanSide = TRUE;
         }
#endif
      }
   }
#ifdef DMP_X_BROADCOM_COM_IPV6_1 /* aka SUPPORT_IPV6 */
   else
   {
      /* ipv6 address */
      char lanAddr6[BUFLEN_48];

      if (oal_getLanAddr6("br0", lanAddr6) != CMSRET_SUCCESS)
      {
         cmsLog_error("cmsDal_getLanAddr6 returns error.");
      }
      else
      {
         char clntAddr6[BUFLEN_48];

         /* see if the client addr is in the same subnet as br0. */
         /* append /64 prefix length to both addresses before comparing them. */
         strcat(lanAddr6, "/64");
         sprintf(clntAddr6, "%s/64", ipAddr);
         onLanSide = cmsNet_areIp6SubnetPrefixEqual(clntAddr6, lanAddr6);
      }
   }
#endif

   return onLanSide;

}  /* End of cmsNet_isAddressOnLanSide() */


/***************************************************************************
// Function Name: cmsNet_getLeftMostOneBits.
// Description  : get the left most one bit number in the given number.
// Parameters   : num -- the given number.
// Returns      : the left most one bit number in the given number.
****************************************************************************/
SINT32 cmsNet_getLeftMostOneBits(SINT32 num) 
{
   int pos = 0;
   int numArr[8] = {128, 64, 32, 16, 8, 4, 2, 1};

   // find the left most zero bit position
   for ( pos = 0; pos < 8; pos++ )
   {
      if ( (num & numArr[pos]) == 0 )
         break;
   }

   return pos;
}

/***************************************************************************
// Function Name: cmsNet_getLeftMostOneBitsInMask.
// Description  : get number of left most one bit in the given subnet mask.
// Parameters   : mask -- subnet mask.
// Returns      : number of left most one bit in subnet mask.
****************************************************************************/
SINT32 cmsNet_getLeftMostOneBitsInMask(char *mask) 
{
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_16];
   int num = 0, total = 0;

   if ( mask == NULL ) 
      return total;

   // need to copy since strtok_r updates string
   strcpy(buf, mask);

   // mask has the following format
   //   xxx.xxx.xxx.xxx where x is decimal number
   pToken = strtok_r(buf, ".", &pLast);
   if ( pToken == NULL ) 
      return total;

   num = cmsNet_getLeftMostOneBits(atoi(pToken));
   total += num;
   while ( num == 8 ) 
   {
      pToken = strtok_r(NULL, ".", &pLast);
      if ( pToken == NULL ) 
      {
         break;
      }
      num = cmsNet_getLeftMostOneBits(atoi(pToken));
      total += num;
   }

   return total;
}

SINT32 cmsNet_getIfindexByIfname(char *ifname)
{
   int sockfd;
   int idx;
   int ifindex = -1;
   struct ifreq ifr;

   /* open socket to get INET info */
   if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) <= 0)
   {
      cmsLog_error("socket returns error. sockfd=%d", sockfd);
      return -1;
   }

   for (idx = 2; idx < 32; idx++)
   {
      memset(&ifr, 0, sizeof(struct ifreq));
      ifr.ifr_ifindex = idx;

      if (ioctl(sockfd, SIOCGIFNAME, &ifr) >= 0)
      {
         if (strcmp(ifname, ifr.ifr_name) == 0)
         {
            ifindex = idx;
            break;
         }
      }
   }
   
   close(sockfd);

   return ifindex;

}  /* End of cmsNet_getIfindexByIfname() */


#ifdef DMP_X_BROADCOM_COM_IPV6_1 /* aka SUPPORT_IPV6 */

CmsRet cmsNet_getIfAddr6(const char *ifname, UINT32 addrIdx,
                         char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags)
{
   return oal_getIfAddr6(ifname, addrIdx, ipAddr, ifIndex, prefixLen, scope, ifaFlags);
}

UBOOL8 cmsNet_areIp6AddrEqual(const char *ip6Addr1, const char *ip6Addr2)
{
   char address1[BUFLEN_40];
   char address2[BUFLEN_40];
   UINT32 plen1 = 0;
   UINT32 plen2 = 0;
   struct in6_addr   in6Addr1, in6Addr2;
   CmsRet ret;

   if (IS_EMPTY_STRING(ip6Addr1) && IS_EMPTY_STRING(ip6Addr2))
   {
      return TRUE;
   }
   if (ip6Addr1 == NULL || ip6Addr2 == NULL)
   {
      return FALSE;
   }

   if ((ret = cmsUtl_parsePrefixAddress(ip6Addr1, address1, &plen1)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return FALSE;
   }
   if ((ret = cmsUtl_parsePrefixAddress(ip6Addr2, address2, &plen2)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return FALSE;
   }

   if (inet_pton(AF_INET6, address1, &in6Addr1) <= 0)
   {
      cmsLog_error("Invalid address1=%s", address1);
      return FALSE;
   }
   if (inet_pton(AF_INET6, address2, &in6Addr2) <= 0)
   {
      cmsLog_error("Invalid address2=%s", address2);
      return FALSE;
   }

   return ((memcmp(&in6Addr1, &in6Addr2, sizeof(struct in6_addr)) == 0) && (plen1 == plen2));

}  /* cmsNet_areIp6AddrEqual() */

UBOOL8 cmsNet_areIp6DnsEqual(const char *dnsServers1, const char *dnsServers2)
{
   char dnsPri1[BUFLEN_40];
   char dnsSec1[BUFLEN_40];
   char dnsPri2[BUFLEN_40];
   char dnsSec2[BUFLEN_40];
   CmsRet ret;

   *dnsPri1 = '\0';
   *dnsSec1 = '\0';
   *dnsPri2 = '\0';
   *dnsSec2 = '\0';

   if (IS_EMPTY_STRING(dnsServers1) && IS_EMPTY_STRING(dnsServers2))
   {
      return TRUE;
   }
   if (dnsServers1 == NULL || dnsServers2 == NULL)
   {
      return FALSE;
   }

   if ((ret = cmsUtl_parseDNS(dnsServers1, dnsPri1, dnsSec1)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parseDNS returns error. ret=%d", ret);
      return FALSE;
   }
   if ((ret = cmsUtl_parseDNS(dnsServers2, dnsPri2, dnsSec2)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parseDNS returns error. ret=%d", ret);
      return FALSE;
   }

   if (!cmsNet_areIp6AddrEqual(dnsPri1, dnsPri2) ||
       !cmsNet_areIp6AddrEqual(dnsSec2, dnsSec2))
   {
      return FALSE;
   }

   return TRUE;

}  /* cmsNet_areIp6DnsEqual() */

UBOOL8 cmsNet_areIp6SubnetPrefixEqual(const char *sp1, const char *sp2)
{
   char address1[BUFLEN_40];
   char address2[BUFLEN_40];
   char prefix1[BUFLEN_40];
   char prefix2[BUFLEN_40];
   UINT32 plen1 = 0;
   UINT32 plen2 = 0;
   CmsRet ret;

   *address1 = '\0';
   *address2 = '\0';
   *prefix1  = '\0';
   *prefix2  = '\0';

   if (IS_EMPTY_STRING(sp1) && IS_EMPTY_STRING(sp2))
   {
      return TRUE;
   }
   if (sp1 == NULL || sp2 == NULL)
   {
      return FALSE;
   }

   if ((ret = cmsUtl_parsePrefixAddress(sp1, address1, &plen1)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return FALSE;
   }
   if ((ret = cmsUtl_parsePrefixAddress(sp2, address2, &plen2)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return FALSE;
   }

   if ((ret = cmsUtl_getAddrPrefix(address1, plen1, prefix1)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_getAddrPrefix returns error. ret=%d", ret);
      return FALSE;
   }
   if ((ret = cmsUtl_getAddrPrefix(address2, plen2, prefix2)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_getAddrPrefix returns error. ret=%d", ret);
      return FALSE;
   }

   return (cmsNet_areIp6AddrEqual(prefix1, prefix2) && (plen1 == plen2));

}  /* cmsNet_areIp6SubnetPrefixEqual() */

CmsRet cmsNet_subnetIp6SitePrefix(const char *sp, UINT8 subnetId, UINT32 snPlen, char *snPrefix)
{
   char prefix[BUFLEN_40];
   char address[BUFLEN_40];
   UINT32 plen;
   struct in6_addr   in6Addr;
   CmsRet ret;

   if (snPrefix == NULL)
   {
      cmsLog_error("snPrefix is NULL.");
      return CMSRET_INVALID_ARGUMENTS;
   }
   *snPrefix = '\0';

   if (IS_EMPTY_STRING(sp))
   {
      cmsLog_error("sp is empty. do nothing.");
      return CMSRET_SUCCESS;
   }

   /* set a limitation to subnet prefix length to be at 8 bit boundary */
   if (snPlen % 8)
   {
      cmsLog_error("snPlen is not at 8 bit boundary. snPlen=%d", snPlen);
      return CMSRET_INVALID_ARGUMENTS;
   }

   if ((ret = cmsUtl_parsePrefixAddress(sp, address, &plen)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_parsePrefixAddress returns error. ret=%d", ret);
      return CMSRET_INVALID_ARGUMENTS;
   }

   if ((snPlen <= plen) || (subnetId >= (1<<(snPlen-plen))))
   {
      cmsLog_error("plen=%d snPlen=%d subnetId=%d", plen, snPlen, subnetId);
      return CMSRET_INVALID_ARGUMENTS;
   }
   
   if ((ret = cmsUtl_getAddrPrefix(address, plen, prefix)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsUtl_getAddrPrefix returns error. ret=%d", ret);
      return ret;
   }

   if (inet_pton(AF_INET6, prefix, &in6Addr) <= 0)
   {
      cmsLog_error("inet_pton returns error");
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* subnet the site prefix */
   in6Addr.s6_addr[(snPlen-1)/8] |= subnetId;

   if (inet_ntop(AF_INET6, &in6Addr, snPrefix, BUFLEN_40) == NULL)
   {
      cmsLog_error("inet_ntop returns error");
      return CMSRET_INTERNAL_ERROR;
   }

   return CMSRET_SUCCESS;

}  /* cmsNet_subnetIp6SitePrefix() */

#endif

