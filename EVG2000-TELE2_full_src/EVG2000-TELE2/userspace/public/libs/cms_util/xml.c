/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
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

#include <string.h>
#include "cms.h"
#include "cms_mem.h"
#include "cms_log.h"


CmsRet cmsXml_escapeString(const char *string, char **escapedString)
{
   UINT32 len, len2, i=0, j=0;
   char *tmpStr;

   if (string == NULL)
   {
      return CMSRET_SUCCESS;
   }

   len = strlen(string);
   len2 = len;

   /* see how many characters need to be escaped and what the new length is */
   while (i < len)
   {
      if (string[i] == '<' || string[i] == '>')
      {
         len2 += 3;
      }
      else if (string[i] == '&' || string[i] == '%')
      {
         len2 += 4;
      }
      i++;
   }

   if ((tmpStr = cmsMem_alloc(len2+1, ALLOC_ZEROIZE)) == NULL)
   {
      cmsLog_error("failed to allocate %d bytes", len+1);
      return CMSRET_RESOURCE_EXCEEDED;
   }

   i=0;
   while (i < len)
   {
      if (string[i] == '<')
      {
         tmpStr[j++] = '&';
         tmpStr[j++] = 'l';
         tmpStr[j++] = 't';
         tmpStr[j++] = ';';
      }
      else if (string[i] == '>')
      {
         tmpStr[j++] = '&';
         tmpStr[j++] = 'g';
         tmpStr[j++] = 't';
         tmpStr[j++] = ';';
      }
      else if (string[i] == '&')
      {
         tmpStr[j++] = '&';
         tmpStr[j++] = 'a';
         tmpStr[j++] = 'm';
         tmpStr[j++] = 'p';
         tmpStr[j++] = ';';
      }
      else if (string[i] == '%')
      {
         tmpStr[j++] = '&';
         tmpStr[j++] = '#';
         tmpStr[j++] = '3';
         tmpStr[j++] = '7';
         tmpStr[j++] = ';';
      }
      else
      {
         tmpStr[j++] = string[i];
      }

      i++;
   }

   *escapedString = tmpStr;

   return CMSRET_SUCCESS;
}


CmsRet cmsXml_unescapeString(const char *escapedString, char **string)
{
   UINT32 len, i=0, j=0;
   char *tmpStr;

   if (escapedString == NULL)
   {
      return CMSRET_SUCCESS;
   }

   len = strlen(escapedString);

   if ((tmpStr = cmsMem_alloc(len+1, ALLOC_ZEROIZE)) == NULL)
   {
      cmsLog_error("failed to allocate %d bytes", len+1);
      return CMSRET_RESOURCE_EXCEEDED;
   }

   while (i < len)
   {
      if (escapedString[i] != '&')
      {
         tmpStr[j++] = escapedString[i++];
      }
      else
      {
         /*
          * We have a possible escape sequence.  Check for the
          * whole thing in 1 shot.
          */
         if ((i+3<len) &&
             escapedString[i+1] == 'g' &&
             escapedString[i+2] == 't' &&
             escapedString[i+3] == ';')
         {
            tmpStr[j++] = '>';
            i += 4;
         }
         else if ((i+3<len) &&
             escapedString[i+1] == 'l' &&
             escapedString[i+2] == 't' &&
             escapedString[i+3] == ';')
         {
            tmpStr[j++] = '<';
            i += 4;
         }
         else if ((i+4<len) &&
             escapedString[i+1] == 'a' &&
             escapedString[i+2] == 'm' &&
             escapedString[i+3] == 'p' &&
             escapedString[i+4] == ';')
         {
            tmpStr[j++] = '&';
            i += 5;
         }
         else if ((i+4<len) &&
             escapedString[i+1] == '#' &&
             escapedString[i+2] == '3' &&
             escapedString[i+3] == '7' &&
             escapedString[i+4] == ';')
         {
            tmpStr[j++] = '%';
            i += 5;
         }
         else
         {
            /* not a valid escape sequence, just copy it */
            tmpStr[j++] = escapedString[i++];
         }
      }
   }

   *string = tmpStr;

   return CMSRET_SUCCESS;
}




