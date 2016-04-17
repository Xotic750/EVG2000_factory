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

#ifndef __CMS_TMR_H__
#define __CMS_TMR_H__


/*!\file cms_tmr.h
 * \brief Header file for the CMS Event Timer API.
 *  This is in the cms_util library.
 *
 */
#include "cms.h"
#include "cms_util.h"


/** Event handler type definition
 */
typedef void (*CmsEventHandler)(void*);


/** Max length (including NULL character) of an event timer name.
 *
 * When an event timer is created, the caller can give it a name
 * to help with debugging and lookup.  Name is optional.
 */
#define CMS_EVENT_TIMER_NAME_LENGTH  32


/** Initialize a timer handle.
 *
 * @param tmrHandle (OUT) On successful return, a handle to be used for
 *                        future handle operation is returned.
 *
 * @return CmsRet enum.
 */
CmsRet cmsTmr_init(void **tmrHandle);


/** Clean up a timer handle, including stopping and deleting all 
 *  unexpired timers and freeing the timer handle itself.
 *
 * @param tmrHandle (IN/OUT) Timer handle returned by cmsTmr_init().
 */
void cmsTmr_cleanup(void **tmrHandle);


/** Create a new event timer which will expire in the specified number of 
 *  milliseconds.
 *
 * Since lookups are done using a combination of the handler func and
 * context data, there must not be an existing timer event in the handle
 * with the same handler func and context data.  (We could allow 
 * multiple entries with the same func and ctxData, but we will have to
 * clarify what it means to cancel a timer, cancel all or cancel the
 * next timer.)
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN)     The handler func.
 * @param ctxData   (IN)     Optional data to be passed in with the handler func.
 * @param ms        (IN)     Timer expiration value in milliseconds.
 * @param name      (IN)     Optional name of this timer event.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_set(void *tmrHandle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);


/** Stop an event timer and delete it.
 *
 * The event timer is found by matching the callback func and ctxData.
 *
 * @param tmrHandle (IN/OUT) Pointer to the event timer handle;
 * @param func      (IN) The event handler.
 * @param *handle   (IN) Argument passed to the event handler.
 */   
void cmsTmr_cancel(void *tmrHandle, CmsEventHandler func, void *ctxData);


/** Get the number of milliseconds until the next event is due to expire.
 *
 * @param tmrHandle (IN)  Pointer to timer handle that was returned by cmsTmr_init().
 * @param ms        (OUT) Number of milliseconds until the next event.
 *
 * @return CmsRet enum.  Specifically, CMSRET_SUCCESS if there is a next event.
 *         If there are no more events in the timer handle, CMSRET_NO_MORE_INSTANCES
 *         will be returned and the parameter ms is set to MAX_UINT32.
 */
CmsRet cmsTmr_getTimeToNextEvent(const void *tmrHandle, UINT32 *ms);


/** Get the number of timer events in the timer handle.
 *
 * @param tmrHandle (IN)  Pointer to timer handle that was returned by cmsTmr_init().
 *
 * @return The number of timer events in the given handle.
 */
UINT32 cmsTmr_getNumberOfEvents(const void *tmrHandle);


/** Execute all events which have expired.
 *
 * This function will call the handler func with the ctxData for all
 * timer events that have expired.  There may be 0, 1, 2, etc. handler
 * functions called by this function.  It is up to the caller of this
 * function to call this function at the appropriate time (using the
 * value of cmsTmr_getTimeToNextEvent() and cmsTmr_getEventCount() as a guide).
 *
 * Once an event is executed, it is deleted and freed.  
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 *
 */
void cmsTmr_executeExpiredEvents(void *tmrHandle);


/** Return true if the specified handler func and context data (event) is set.
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN) The handler func.
 * @param ctxData   (IN) Optional data to be passed in with the handler func.
 *
 * @return TRUE if specified event is present, otherwise, FALSE.
 */   
UBOOL8 cmsTmr_isEventPresent(const void *tmrHandle, CmsEventHandler func, void *ctxData);


/** Use debug logging to dump out all timers in the timer handle.
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 *
 */   
void cmsTmr_dumpEvents(const void *tmrHandle);


#endif  /* __CMS_TMR_H__ */
