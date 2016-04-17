/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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
#include "cms_ast.h"
#include "oal.h"
#include "bget.h"


/** Macro to round up to nearest 4 byte length */
#define ROUNDUP4(s)  (((s) + 3) & 0xfffffffc)


/** Macro to calculate how much we need to allocate for a given user size request.
 *
 * We need the header even when not doing MEM_DEBUG to keep
 * track of the size and allocFlags that was passed in during cmsMem_alloc.
 * This info is needed during cmsMem_realloc.
 */
#ifdef CMS_MEM_DEBUG
#define REAL_ALLOC_SIZE(s) (CMS_MEM_HEADER_LENGTH + ROUNDUP4(s) + CMS_MEM_FOOTER_LENGTH)
#else
#define REAL_ALLOC_SIZE(s) (CMS_MEM_HEADER_LENGTH + (s))
#endif


static CmsMemStats mStats;

#ifdef MDM_SHARED_MEM

/** Macro to determine if a pointer is in shared memory or not. */
#define IS_IN_SHARED_MEM(p) ((((UINT32) (p)) >= mStats.shmAllocStart) && \
                             (((UINT32) (p)) < mStats.shmAllocEnd))


void cmsMem_initSharedMem(void *addr, UINT32 len)
{
   mStats.shmAllocStart = (UINT32) addr;
   mStats.shmAllocEnd = mStats.shmAllocStart + len;
   mStats.shmTotalBytes = len;

   cmsLog_notice("shm pool: %p-%p", mStats.shmAllocStart, mStats.shmAllocEnd);

   bpool(addr, len);
}

void cmsMem_initSharedMemPointer(void *addr, UINT32 len)
{
   /*
    * You might be tempted to do a memset(&mStats, 0, sizeof(mStats)) here.
    * But don't do it.  mStats will be initialized to all zeros anyways since
    * it is in the bss.  And smd will start using the memory allocator before
    * it calls cmsMem_initSharedMemPointer, so if we zero out the structure
    * at the beginning of this function, the counters will be wrong.
    */
   mStats.shmAllocStart = (UINT32) addr;
   mStats.shmAllocEnd = mStats.shmAllocStart + len;
   mStats.shmTotalBytes = len;

   cmsLog_notice("shm pool: %p-%p", mStats.shmAllocStart, mStats.shmAllocEnd);

   bcm_secondary_bpool(addr);
}

#endif

void cmsMem_cleanup(void)
{

#ifdef MDM_SHARED_MEM
   bcm_cleanup_bpool();
#endif

   return;
}


void *cmsMem_alloc(UINT32 size, UINT32 allocFlags)
{
   void *buf;
   UINT32 allocSize;

   allocSize = REAL_ALLOC_SIZE(size);

#ifdef MDM_SHARED_MEM
   if (allocFlags & ALLOC_SHARED_MEM)
   {
      buf = bget(allocSize);
   }
   else
#endif
   {
      buf = oal_malloc(allocSize);
      if (buf)
      {
         mStats.bytesAllocd += size;
         mStats.numAllocs++;
      }
   }


   if (buf != NULL)
   {
      UINT32 *intBuf = (UINT32 *) buf;
      UINT32 intSize = allocSize / sizeof(UINT32);


      if (allocFlags & ALLOC_ZEROIZE)
      {
         memset(buf, 0, allocSize);
      }
#ifdef CMS_MEM_POISON_ALLOC_FREE
      else
      {
         /*
          * Set alloc'ed buffer to garbage to catch use-before-init.
          * But we also allocate huge buffers for storing image downloads.
          * Don't bother writing garbage to those huge buffers.
          */
         if (allocSize < 64 * 1024)
         {
            memset(buf, CMS_MEM_ALLOC_PATTERN, allocSize);
         }
      }
#endif
         
      /*
       * Record the allocFlags in the first word, and the 
       * size of user buffer in the next 2 words of the buffer.
       * Make 2 copies of the size in case one of the copies gets corrupted by
       * an underflow.  Make one copy the XOR of the other so that there are
       * not so many 0's in size fields.
       */
      intBuf[0] = allocFlags;
      intBuf[1] = size;
      intBuf[2] = intBuf[1] ^ 0xffffffff;

      buf = &(intBuf[3]); /* this gets returned to user */

#ifdef CMS_MEM_DEBUG
      {
         UINT8 *charBuf = (UINT8 *) buf;
         UINT32 i, roundup4Size = ROUNDUP4(size);

         for (i=size; i < roundup4Size; i++)
         {
            charBuf[i] = CMS_MEM_FOOTER_PATTERN & 0xff;
         }

         intBuf[intSize - 1] = CMS_MEM_FOOTER_PATTERN;
         intBuf[intSize - 2] = CMS_MEM_FOOTER_PATTERN;
      }
#endif

   }

   return buf;
}


void *cmsMem_realloc(void *origBuf, UINT32 size)
{
   void *buf;
   UINT32 origSize, origAllocSize, origAllocFlags;
   UINT32 allocSize;
   UINT32 *intBuf;

   if (origBuf == NULL)
   {
      cmsLog_error("cannot take a NULL buffer");
      return NULL;
   }

   if (size == 0)
   {
      cmsMem_free(origBuf);
      return NULL;
   }

   allocSize = REAL_ALLOC_SIZE(size);


   intBuf = (UINT32 *) (((UINT32) origBuf) - CMS_MEM_HEADER_LENGTH);

   origAllocFlags = intBuf[0];
   origSize = intBuf[1];

   /* sanity check the original length */
   if (intBuf[1] != (intBuf[2] ^ 0xffffffff))
   {
      cmsLog_error("memory underflow detected, %d %d", intBuf[1], intBuf[2]);
      cmsAst_assert(0);
      return NULL;
   }

   origAllocSize = REAL_ALLOC_SIZE(origSize);

   if (allocSize <= origAllocSize)
   {
      /* currently, I don't shrink buffers, but could in the future. */
      return origBuf;
   }

   buf = cmsMem_alloc(allocSize, origAllocFlags);
   if (buf != NULL)
   {
      /* got new buffer, copy orig buffer to new buffer */
      memcpy(buf, origBuf, origSize);
      cmsMem_free(origBuf);
   }
   else
   {
      /*
       * We could not allocate a bigger buffer.
       * Return NULL but leave the original buffer untouched.
       */
   }

   return buf;
}



/** Free previously allocated memory
 * @param buf Previously allocated buffer.
 */
void cmsMem_free(void *buf)
{
   UINT32 size;

   if (buf != NULL)
   {
      UINT32 *intBuf = (UINT32 *) (((UINT32) buf) - CMS_MEM_HEADER_LENGTH);

      size = intBuf[1];

      if (intBuf[1] != (intBuf[2] ^ 0xffffffff))
      {
         cmsLog_error("memory underflow detected, %d %d", intBuf[1], intBuf[2]);
         cmsAst_assert(0);
         return;
      }

#ifdef CMS_MEM_DEBUG
      {
         UINT32 allocSize, intSize, roundup4Size, i;
         UINT8 *charBuf = (UINT8 *) buf;

         allocSize = REAL_ALLOC_SIZE(intBuf[1]);
         intSize = allocSize / sizeof(UINT32);
         roundup4Size = ROUNDUP4(intBuf[1]);

         for (i=intBuf[1]; i < roundup4Size; i++)
         {
            if (charBuf[i] != (UINT8) (CMS_MEM_FOOTER_PATTERN & 0xff))
            {
               cmsLog_error("memory overflow detected at idx=%d 0x%x 0x%x 0x%x",
                            i, charBuf[i], intBuf[intSize-1], intBuf[intSize-2]);
               cmsAst_assert(0);
               return;
            }
         }
               
         if ((intBuf[intSize - 1] != CMS_MEM_FOOTER_PATTERN) ||
             (intBuf[intSize - 2] != CMS_MEM_FOOTER_PATTERN))
         {
            cmsLog_error("memory overflow detected, 0x%x 0x%x",
                         intBuf[intSize - 1], intBuf[intSize - 2]);
            cmsAst_assert(0);
            return;
         }

#ifdef CMS_MEM_POISON_ALLOC_FREE
         /*
          * write garbage into buffer which is about to be freed to detect
          * users of freed buffers.
          */
         memset(intBuf, CMS_MEM_FREE_PATTERN, allocSize);
#endif
      }

#endif  /* CMS_MEM_DEBUG */

      buf = intBuf;  /* buf points to real start of buffer */


#ifdef MDM_SHARED_MEM
      if (IS_IN_SHARED_MEM(buf))
      {
         brel(buf);
      }
      else
#endif
      {
         oal_free(buf);
         mStats.bytesAllocd -= size;
         mStats.numFrees++;
      }
   }
}


char *cmsMem_strdup(const char *str)
{
   return cmsMem_strdupFlags(str, 0);
}


char *cmsMem_strdupFlags(const char *str, UINT32 flags)
{
   UINT32 len;
   void *buf;

   if (str == NULL)
   {
      return NULL;
   }

   /* this is somewhat dangerous because it depends on str being NULL
    * terminated.  Use strndup/strlen if not sure the length of the string.
    */
   len = strlen(str);

   buf = cmsMem_alloc(len+1, flags);
   if (buf == NULL)
   {
      return NULL;
   }

   strncpy((char *) buf, str, len+1);

   return ((char *) buf);
}



char *cmsMem_strndup(const char *str, UINT32 maxlen)
{
   return cmsMem_strndupFlags(str, maxlen, 0);
}


char *cmsMem_strndupFlags(const char *str, UINT32 maxlen, UINT32 flags)
{
   UINT32 len;
   char *buf;

   if (str == NULL)
   {
      return NULL;
   }

   len = cmsMem_strnlen(str, maxlen, NULL);

   buf = (char *) cmsMem_alloc(len+1, flags);
   if (buf == NULL)
   {
      return NULL;
   }

   strncpy(buf, str, len);
   buf[len] = 0;

   return buf;
}


UINT32 cmsMem_strnlen(const char *str, UINT32 maxlen, UBOOL8 *isTerminated)
{
   UINT32 len=0;

   while ((len < maxlen) && (str[len] != 0))
   {
      len++;
   }

   if (isTerminated != NULL)
   {
      *isTerminated = (str[len] == 0);
   }

   return len;
}


void cmsMem_getStats(CmsMemStats *stats)
{
   bufsize curalloc, totfree, maxfree;
   long nget, nrel;


   stats->shmAllocStart = mStats.shmAllocStart;
   stats->shmAllocEnd = mStats.shmAllocEnd;
   stats->shmTotalBytes = mStats.shmTotalBytes;

   /*
    * Get shared memory stats from bget.  The shared memory stats reflects
    * activity of all processes who are accessing the shared memory region,
    * not just this process.
    */
   bstats(&curalloc, &totfree, &maxfree, &nget, &nrel);

   stats->shmBytesAllocd = (UINT32) curalloc;
   stats->shmNumAllocs = (UINT32) nget;
   stats->shmNumFrees = (UINT32) nrel;


   /* the private heap memory stats can come directly from our data structure */
   stats->bytesAllocd = mStats.bytesAllocd;
   stats->numAllocs = mStats.numAllocs;
   stats->numFrees = mStats.numFrees;

   return;
}



