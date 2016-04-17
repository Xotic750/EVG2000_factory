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

#ifndef __CMS_TMS_H__
#define __CMS_TMS_H__


/*!\file cms_tms.h
 * \brief Header file for the CMS Timestamp API.
 *  This is in the cms_util library.
 *
 */

#include "cms.h"

/** Number of nanoseconds in 1 second. */
#define NSECS_IN_SEC 1000000000

/** Number of nanoseconds in 1 milli-second. */
#define NSECS_IN_MSEC 1000000

/** Number of nanoseconds in 1 micro-second. */
#define NSECS_IN_USEC 1000

/** Number of micro-seconds in 1 second. */
#define USECS_IN_SEC  1000000

/** Number of micro-seconds in a milli-second. */
#define USECS_IN_MSEC 1000

/** Number of milliseconds in 1 second */
#define MSECS_IN_SEC  1000




/** OS independent timestamp structure.
 */
typedef struct
{
   UINT32 sec;   /**< Number of seconds since some arbitrary point. */
   UINT32 nsec;  /**< Number of nanoseconds since some arbitrary point. */
} CmsTimestamp;



/** Get the current timestamp.
 * 
 * The timestamps are supposed to be unaffected by changes in the system
 * time.  Even though timestamps have a nanosecond field, the resolution
 * of the timetime is probably around 2.5 to 10ms.
 *
 *@param tms (IN) This structure is filled with the current timestamp.
 */   
void cmsTms_get(CmsTimestamp *tms);


/** Calculate the difference between two timestamps.
 *
 * This function correctly handles rollover of the timestamps.
 * That is, if newTms={0.0} and oldTms={4294967295,999999000} (1000ns before
 * rollover) then deltaTms={0,1000}.
 * Therefore,  caller must be careful to specify the two timestamps for
 * this function in the correct order.  Otherwise, the function will
 * think that a rollover has occured and return a surpringly large delta.
 *
 * In the more common, non-rollover case, example would be
 * newTms={10,500000000} and oldTms={3,100000000} so
 * deltaTms={7,400000000}.
 *
 * @param newTms    (IN) The timestamp that was obtained more recently.
 * @param oldTms    (IN) The timestamp that was obtained in the past.
 * @param deltaTms (OUT) The difference between newTms and oldTms.
 */   
void cmsTms_delta(const CmsTimestamp *newTms,
                  const CmsTimestamp *oldTms,
                  CmsTimestamp *deltaTms);


/** Calculate the difference between two timestamps and return their
 *  difference in milliseconds.
 *
 * This function uses cmsTms_delta(), so the comments for that function
 * applies to this function as well.
 *
 * @param newTms    (IN) The timestamp that was obtained more recently.
 * @param oldTms    (IN) The timestamp that was obtained in the past.
 * @return delta in milli-seconds.  If the delta in milliseconds is too
 *         large for an UINT32, then MAX_UINT32 is returned.
 */   
UINT32 cmsTms_deltaInMilliSeconds(const CmsTimestamp *newTms,
                                  const CmsTimestamp *oldTms);


/** Add the specified number of milliseconds to the given timestamp.
 *
 * @param tms    (IN/OUT) The timestamp to operate on.
 * @param ms     (IN)     The number of millisconds to add to tms.
 */
void cmsTms_addMilliSeconds(CmsTimestamp *tms, UINT32 ms);


/** Format the specified time in XSI format for TR69.
 *
 * @param t      (IN) Number of seconds since Jan 1 1970.  If 0, then this
 *                    function will use the current time.
 * @param buf   (OUT) Buffer to hold the formatted time.
 * @param bufLen (IN) Length of buffer.
 *
 * @return CmsRet enum, specifically, if the buffer is not big enough,
 *                      CMSRET_RESOURCE_EXCEEDED will be returned.
 */
CmsRet cmsTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen);

#endif  /* __CMS_TMS_H__ */
