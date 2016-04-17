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


#include <stdlib.h>
#include <sys/shm.h> /* for shmat */
#include <sys/sem.h> /* for struct sembuf */
#include "../oal.h"


/*
 * Some of these defines are duplicates of those in busybox/sysklogd, so
 * look there before changing anything here.
 */

static const long KEY_ID = 0x414e4547; /*"GENA"*/
static struct shbuf_ds {
	int size;		// size of data written
	int head;		// start of message list
	int tail;		// end of message list
	char data[1];		// data/messages
} *buf = NULL;			// shared memory pointer
// Semaphore operation structures
static struct sembuf SMrup[1] = {{0, -1, IPC_NOWAIT | SEM_UNDO}}; // set SMrup
static struct sembuf SMrdn[2] = {{1, 0, 0}, {0, +1, SEM_UNDO}}; // set SMrdn
static int	log_shmid = -1;	// ipc shared memory id
static int	log_semid = -1;	// ipc semaphore id

#define BCM_SYSLOG_MESSAGE_LEN_BYTES    4


/*
 * sem_up - up()'s a semaphore.
 */
static inline void sem_up(int semid)
{
	if ( semop(semid, SMrup, 1) == -1 ) 
		cmsLog_error("semop[SMrup]");
}

/*
 * sem_down - down()'s a semaphore
 */				
static inline void sem_down(int semid)
{
	if ( semop(semid, SMrdn, 2) == -1 )
		cmsLog_error("semop[SMrdn]");
}

int oal_readLogPartial(int ptr, char* buffer)
{
  int i=BCM_SYSLOG_READ_BUFFER_ERROR;
  int len;
  int end=0;

  if ( (log_shmid = shmget(KEY_ID, 0, 0)) == -1) {
    cmsLog_debug("Syslog disabled or log buffer not allocated\n");
    goto output_end;
  }
  // Attach shared memory to our char*
  if ( (buf = shmat(log_shmid, NULL, SHM_RDONLY)) == NULL) {
    cmsLog_error("Can't get access to circular buffer from syslogd\n");
    end = 1;
    goto output_end;
  }
  
  if ( (log_semid = semget(KEY_ID, 0, 0)) == -1) {
    cmsLog_error("Can't get access to semaphone(s) for circular buffer from syslogd\n");
    end = 1;
    goto output_end;
  }
  
  sem_down(log_semid);
  // Read Memory
  if (ptr == BCM_SYSLOG_FIRST_READ)
    i = buf->head;
  else
    i = ptr;
  if (buf->head == buf->tail) {
    cmsLog_debug("<empty syslog buffer>\n");
    i = BCM_SYSLOG_READ_BUFFER_END;
    end = 1;
    goto nothing2display;
  }

readnext:
  if ( i != buf->tail) {
    if (i >= buf->size )
      i = 0;
    snprintf(buffer, BCM_SYSLOG_MAX_LINE_SIZE, "%s", buf->data+i);
    i += strlen(buf->data+i) + 1;
    if (i >= buf->size )
      i = 0;
    len = strlen(buffer);
    if (!((buffer[len] == '\0') &&
      (buffer[len-1] == '\n'))) {
        snprintf(&buffer[len], BCM_SYSLOG_MAX_LINE_SIZE-len, "%s", buf->data+i);
        len = strlen(buffer);
        i += strlen(buf->data+i) + 1;
        if (i >= buf->size )
          i = 0;
      }
    /* work around for syslogd.c bug which generate first log without timestamp */
    if (strlen(buffer) < 16 || buffer[3] != ' ' || buffer[6] != ' ' ||
      buffer[9] != ':' || buffer[12] != ':' || buffer[15] != ' ') {
        goto readnext;
      }
    buffer[len-BCM_SYSLOG_MESSAGE_LEN_BYTES-1] = '\n';
    buffer[len-BCM_SYSLOG_MESSAGE_LEN_BYTES] = '\0';
  }
  else {
    /* read to the end already */
    i = BCM_SYSLOG_READ_BUFFER_END;
    end = 1;
  }

nothing2display:
  sem_up(log_semid);

output_end:
  if (log_shmid != -1)
    shmdt(buf);

  if (end) {
    i=BCM_SYSLOG_READ_BUFFER_END;
    buffer[0]='\0';
  }
  return i;
}

