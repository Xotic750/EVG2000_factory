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

#ifndef __CMS_BOARDIOCTL_H__
#define __CMS_BOARDIOCTL_H__

#include <sys/ioctl.h>
#include "cms.h"
#include "board.h"  /* in bcmdrivers/opensource/include/bcm963xx for BOARD_IOCTL_ACTION */


/*!\file cms_boardioctl.h
 * \brief Header file for the Board Control API, board ioctl portion.
 *
 * This file is separate from all the other commands because if a program
 * wants to call devCtl_boardIoctl, it needs to include a bunch of header
 * files from the bcm kernel driver directories.  Most apps will not need
 * to call the boardIoctl directly.
 *
 */

/** Device filename of ioctl device. */
#define BOARD_DEVICE_NAME  "/dev/brcmboard"


/** Do board ioctl.
 *
 * @param boardIoctl (IN) The ioctl to perform.
 * @param action     (IN) The sub-action associated with the ioctl.
 * @param string     (IN) Input data for the ioctl.
 * @param strLen     (IN) Length of input data.
 * @param offset     (IN) Offset for the ioctl/sub-action.
 * @param data   (IN/OUT) Depends on the ioctl/sub-action.  Could be used
 *                        to pass in additional data or be used to return data
 *                        from the ioctl.
 * @return CmsRet enum.
 */
CmsRet devCtl_boardIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data);


#endif /* __CMS_BOARDIOCTL_H__ */
