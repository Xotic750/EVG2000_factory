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


#ifndef __CMS_FIL_H__
#define __CMS_FIL_H__

/*!\file cms_fil.h
 * \brief Header file for file and directory utilities functions.
 */


/** Return true if the filename exists.
 *
 * @param filename (IN) full pathname to the file.
 *
 * @return TRUE if the specified file exists.
 */
UBOOL8 cmsFil_isFilePresent(const char *filename);



/** Return the size of the file.
 *
 * @param filename (IN) full pathname to the file.
 *
 * @return -1 if the file does not exist, otherwise, the file size.
 */
SINT32 cmsFil_getSize(const char *filename);



/** Copy the contents of the file to the specified buffer.
 *
 * @param filename (IN) full pathname to the file.
 * @param buf     (OUT) buffer that will hold contents of the file.
 * @bufSize    (IN/OUT) On input, the size of the buffer, on output
 *                      the actual number of bytes that was copied
 *                      into the buffer.
 *
 * @return CmsRet enum.
 */
CmsRet cmsFil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize);



#endif /* __CMS_FIL_H__ */
