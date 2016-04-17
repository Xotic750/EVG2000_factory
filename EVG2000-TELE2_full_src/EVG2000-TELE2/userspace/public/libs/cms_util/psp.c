/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
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

#include "cms.h"
#include "cms_util.h"
#include "cms_boardioctl.h"

CmsRet cmsPsp_set(const char *key, const void *buf, UINT32 bufLen)
{
   char *currBuf;
   SINT32 count;

   if ((currBuf = cmsMem_alloc(bufLen, 0)) == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /*
    * Writing to the scratch pad is a non-preemptive time consuming
    * operation that should be avoided.
    * Check if the new data is the same as the old data.
    */
   count = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
                           (char *) key, 0, (SINT32) bufLen, currBuf);
   if (count == (SINT32) bufLen)
   {
      if (memcmp(currBuf, buf, bufLen) == 0)
      {
         cmsMem_free(currBuf);
         /* set is exactly the same as the orig data, no set needed */
         return CMSRET_SUCCESS;
      }

      cmsMem_free(currBuf);
   }
      

   return (devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD,
                             (char *) key, 0, (SINT32) bufLen, (void *) buf));
}


SINT32 cmsPsp_get(const char *key, void *buf, UINT32 bufLen)
{

   return ((SINT32) devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
                                      (char *) key, 0, (SINT32) bufLen, buf));
}


SINT32 cmsPsp_list(char *buf, UINT32 bufLen)
{

   return ((SINT32) devCtl_boardIoctl(BOARD_IOCTL_FLASH_LIST, SCRATCH_PAD,
                                      NULL, 0, (SINT32) bufLen, buf));
}


CmsRet cmsPsp_clearAll(void)
{

   return (devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD,
                             "", -1, -1, ""));

}



