/* vi: set sw=4 ts=4: */
/*
 * Mini reboot implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>
#include "busybox.h"
#include "init_shared.h"


extern int reboot_main(int argc, char **argv)
{
	char *delay; /* delay in seconds before rebooting */

    /* Foxconn added start, by EricHuang, 08/27/2008 */
    FILE *fp = NULL;
    char line[256];
    int  pid;
    
    if (fp = fopen ("/var/run/ensip.pid", "r"), fp)
    {
        int pid;
        fread (line, 100, 1, fp);
        pid = atoi(line);
        if (pid > 0)
        {
            kill(pid, SIGILL); /* kill brcm's endpoint driver first */
            sleep(8);
            kill(pid, SIGINT);
            sleep(3);
        }
        fclose (fp);
        fp = NULL;
    }
    /* Foxconn added end, by EricHuang, 08/27/2008 */

	if(bb_getopt_ulflags(argc, argv, "d:", &delay)) {
		sleep(atoi(delay));
	}

#ifndef CONFIG_INIT
#ifndef RB_AUTOBOOT
#define RB_AUTOBOOT		0x01234567
#endif
	return(bb_shutdown_system(RB_AUTOBOOT));
#else
	return kill_init(SIGTERM);
#endif
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/