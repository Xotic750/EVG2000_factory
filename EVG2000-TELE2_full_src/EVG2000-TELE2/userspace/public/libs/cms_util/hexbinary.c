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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cms_util.h"


CmsRet cmsUtl_binaryBufToHexString(const UINT8 *binaryBuf, UINT32 binaryBufLen, char **hexStr)
{
   UINT32 i, j;

   if (hexStr == NULL)
   {
      cmsLog_error("hexStr buffer is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }

   *hexStr = cmsMem_alloc((binaryBufLen*2)+1, ALLOC_ZEROIZE);
   if (*hexStr == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   for (i=0, j=0; i < binaryBufLen; i++, j+=2)
   {
      sprintf(&((*hexStr)[j]), "%02x", binaryBuf[i]);
   }

   return CMSRET_SUCCESS;
}


CmsRet cmsUtl_hexStringToBinaryBuf(const char *hexStr, UINT8 **binaryBuf, UINT32 *binaryBufLen)
{
   UINT32 len;
   UINT32 val;
   UINT32 i, j;
   char tmpbuf[3];
   CmsRet ret;

   len = strlen(hexStr);
   if (len % 2 != 0)
   {
      cmsLog_error("hexStr must be an even number of characters");
      return CMSRET_INVALID_ARGUMENTS;
   }

   *binaryBuf = cmsMem_alloc(len/2, 0);
   if (*binaryBuf == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   for (i=0, j=0; j < len; i++, j+=2)
   {
      tmpbuf[0] = hexStr[j];
      tmpbuf[1] = hexStr[j+1];
      tmpbuf[2] = 0;

      ret = cmsUtl_strtoul(tmpbuf, NULL, 16, &val);
      if (ret != CMSRET_SUCCESS)
      {
         cmsMem_free(*binaryBuf);
         *binaryBuf = NULL;
         return ret;
      }
      else
      {
         (*binaryBuf)[i] = (UINT8) val;
      }
   }

   /* if we get here, we were successful, set length */
   *binaryBufLen = len / 2;

   return ret;
}



