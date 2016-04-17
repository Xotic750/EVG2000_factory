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


#ifndef __OAL_H__
#define __OAL_H__


#include "cms.h"
#include "cms_eid.h"
#include "cms_msg.h"


/** This is the internal structure of the message handle.
 *
 * It is highly OS dependent.  Management applications should not
 * use any of its fields directly.
 *
 */
typedef struct
{
   CmsEntityId  eid;        /**< Entity id of the owner of this handle. */
   SINT32       commFd;     /**< communications fd */
   UBOOL8       standalone; /**< are we running without smd, for unittests */
   CmsMsgHeader *putBackQueue;  /**< Messages pushed back into the handle. */
} CmsMsgHandle;



/** Initialize messaging system.
 *
 * Same semantics as cmsMsg_init().
 * 
 * @param eid       (IN)  Entity id of the calling process.
 * @param msgHandle (OUT) msgHandle.
 *
 * @return CmsRet enum.
 */
CmsRet oalMsg_init(CmsEntityId eid, void **msgHandle);


/** Clean up messaging system.
 *
 * Same semantics as cmsMsg_cleanup().
 * @param msgHandle (IN) This was the msg_handle that was
 *                       created by cmsMsg_init().
 */
void oalMsg_cleanup(void **msgHandle);



/** Send a message (blocking).
 *
 * Same semantics as cmsMsg_send().
 *
 * @param fd        (IN) The commFd to send the msg out of.
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 * @return CmsRet enum.
 */
CmsRet oalMsg_send(SINT32 fd, const CmsMsgHeader *buf);


/** Receive a new message from fd.
 *
 * @param fd         (IN) commFd to receive input from.
 * @param buf       (OUT) Returns a pointer to message buffer.  Caller is responsible
 *                        for freeing the buffer.
 * @param timeout    (IN) Pointer to UINT32, specifying the timeout in milliseconds.
 *                        If pointer is NULL, receive will block until a message is
 *                        received.
 *
 * @return CmsRet enum.
 */
CmsRet oalMsg_receive(SINT32 fd, CmsMsgHeader **buf, UINT32 *timeout);


/** Get operating system dependent handle for receive message notification.
 *
 * Same semantics as cmsMsg_getEventHandle();
 * @param msgHandle    (IN) This was the msgHandle created by cmsMsg_init().
 * @param eventHandle (OUT) This is the OS dependent event handle.  For LINUX,
 *                          eventHandle is the file descriptor number.
 * @return CmsRet enum.
 */
CmsRet oalMsg_getEventHandle(const CmsMsgHandle *msgHandle, void *eventHandle);



#endif /* __OAL_H__ */

