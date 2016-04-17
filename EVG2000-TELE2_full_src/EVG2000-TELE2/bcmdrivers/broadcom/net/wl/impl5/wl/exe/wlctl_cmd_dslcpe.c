/*
 * Linux port of wl command line utility
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <typedefs.h>
#include "wlu_linux_dslcpe.h"

static void wlctl_ConvertStrFromShellStr(char *str, char *buf)
{
   if ( buf == NULL ) return;

   int len = strlen(str);
   int i = 0, j = 0;

   for ( i = 0; i < len; i++) {
      if ( str[i] == '\'' || str[i] == '"' ) {
         buf[j++] = str[++i];
         i++;         
      }      
   }

   buf[j]  = '\0';
}

void
wlctl_cmd(char *cmd)
{
	int argc = 0;
	char buf[256], strbuf[256];	
	char *argv[32] ={0};
	char *ptr, *nextptr, *tmpptr;
	int fstdout=-1, fstderr=-1;
	bool outfound = FALSE, errfound = FALSE;
	bool needconvert;
	char outname[64]="", errname[64]="";
	
	if(strlen(cmd) >= 255) {
		fprintf(stderr, "%s: cmd buffer not enough\n", argv[0]);
		return;					
	}
			
	memcpy(buf, cmd, strlen(cmd));
	buf[strlen(cmd)]='\0';
#ifdef DSLCPE_VERBOSE
	printf("%s\n", cmd);
#endif	
	ptr = buf;
	/* build argc, argv */	
	while (*ptr!='\0') {		
		/* skip white space */
		while((*ptr) && *ptr == ' ') { ptr++; };
		/* hardcode some common io redir */
		if(!strncmp(ptr,"2>", strlen("2>"))) {
			ptr += strlen("2>");
			errfound = TRUE;
		}
				
		if(*ptr=='>') {			
			outfound = TRUE;
			ptr++;
		}

		if(!strncmp(ptr,"2>&1", strlen("2>&1"))) {
			ptr += strlen("2>&1");
			errfound = TRUE;
		}					
		
		/* skip white space */			
		while((*ptr) && *ptr == ' ') { ptr++; };
		
		needconvert = FALSE;	
		tmpptr = ptr;
		
		/* search special strings converted by BCMWL_STR_CONVERT */
		while((*tmpptr == '\''|| *tmpptr == '"')) {
			needconvert = TRUE;
			tmpptr += 2;  /* skip ' and this char */
			tmpptr++; /* to next ' or " */
		}		

		nextptr = strchr(tmpptr,' ');
		if(nextptr) {
			*nextptr = '\0';
		}			
					
		if(needconvert) {
			strcpy(strbuf, ptr);
			/* the converted string length should be less than the original*/
			wlctl_ConvertStrFromShellStr(strbuf, ptr);
		}
							
		if(!outfound && !errfound) {							
			argv[argc++] = ptr;
		}
		if(outfound) {
			strcpy(outname, ptr);
			outfound = FALSE; 
		}
		if(errfound) {
			if(*outname) {
				strcpy(errname, outname);
			} else if(strlen(ptr)) {
				strcpy(errname, ptr);
			} else {
				strcpy(errname, "/dev/tty");	
			}			
			errfound = FALSE; 
		}					
		if(nextptr) {ptr = nextptr+1;} else break;
	}
	if(strcmp(argv[0],"wlctl") && strcmp(argv[0],"wl")) {
		fprintf(stderr, "%s: command not found\n", argv[0]);
		return;		
	} 
	
	/* redirect output */
	if(*outname) {
		fflush(stdout);
		fstdout = dup(fileno(stdout));
		freopen(outname, "w", stdout);    
	}
	if(*errname) {
		fflush(stderr);
		fstderr = dup(fileno(stderr));
		freopen(errname, "w", stderr);    
	}		
	
	/* execute cmd */
	wl_libmain(argc, argv);
	
	/* redirect back*/		
	if(fstdout != -1) {
		fflush(stdout);
		dup2(fstdout, fileno(stdout));
		close(fstdout);
		clearerr(stdout);
	}

	if(fstderr != -1) {
		fflush(stderr);
		dup2(fstderr, fileno(stderr));
		close(fstderr);
		clearerr(stderr);
	}
}
