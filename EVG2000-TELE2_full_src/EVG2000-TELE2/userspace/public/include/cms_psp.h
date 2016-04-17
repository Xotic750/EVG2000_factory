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

#ifndef __CMS_PSP_H__
#define __CMS_PSP_H__


/*!\file cms_psp.h
 * \brief Header file for the CMS Persistent Scratch Pad API.
 *  This is in the cms_util library.
 *
 * Persistent storage can be reserved and used by applications on a
 * first-come-first-served basis, meaning if applications request
 * more scratch pad area then available on the system, the later
 * requests will be denied.  
 *
 * On "top-boot" systems, there is 8KB of persistent scratch pad area
 * available.  On "bottom-boot" systems, there is no scratch pad area
 * available.  
 */

#include "cms.h"




/** Write data to persistent scratch pad.
 *
 * This function can be called multiple times for the same key.  If
 * a subsequent call with the same key has a larger data buffer,
 * the region in the scratch pad is grown if there is enough space.
 * If a subsequent call with the same key has a smaller data buffer,
 * the region in the scratch pad is shrunk and the extra data from
 * the previous, larger set is freed.  To delete the key and its
 * storage area from the scratch pad, call this function with the
 * key name with len set to 0.
 *
 * @param key (IN)  A string identifying the scratch pad area.  The key can
 *                  have a maximum of 15 characters.
 * @param buf (IN)  The data to write to the scratch pad.
 * @param bufLen (IN)  Length of Data.
 *
 * @return CmsRet.
 */
CmsRet cmsPsp_set(const char *key, const void *buf, UINT32 bufLen);


/** Read data from persistent scratch pad.
 *
 * @param key (IN)  A string identifying the scratch pad area.  The key can
 *                  have a maximum of 15 characters.
 * @param buf (IN/OUT) User allocates a buffer to hold the read results and
 *                     passes it into this function.  On successful return,
 *                     the buffer contains data that was read.
 * @param bufLen (IN)  Length of given buffer.
 *
 * @return On success, the number of bytes returned to caller.
 *         If the key was not found or there is some other erro, 0 will be returned.
 *         If the key was found but the user provided
 *         buffer is not big enough to hold the data, a negative number will
 *         be returned and the caller's buffer is not modified at all;
 *         the absolute value of the negative number is the 
 *         number of bytes needed to hold the data.
 */
SINT32 cmsPsp_get(const char *key, void *buf, UINT32 bufLen);


/** Get a list of all keys in the persistent scratch pad.
 *
 * @param buf (IN/OUT) User allocates a buffer to hold the read results and
 *                     passes it into this function.  On successful return,
 *                     the buffer contains a list of all keys in the 
 *                     persistent scratch pad.  Each key is terminated by
 *                     a NULL byte.
 *                     
 * @param bufLen (IN)  Length of given buffer.
 *
 * @return On success, the number of bytes returned to caller.
 *         If the persistent scratch pad is empty, 0 will be returned.
 *         If the user provided buffer is not big enough to hold all the keys,
 *         a negative number will be returned and the caller's buffer is not modified at all;
 *         the absolute value of the negative number is the 
 *         number of bytes needed to hold the data.
 */
SINT32 cmsPsp_list(char *buf, UINT32 bufLen);


/** Zeroize all keys from scratch pad area.
 *
 * This has the effect of clearing the entire scratch pad.
 * Use with extreme caution.
 *
 * @return CmsRet enum.
 */
CmsRet cmsPsp_clearAll(void);


#endif /* __CMS_PSP_H__ */
