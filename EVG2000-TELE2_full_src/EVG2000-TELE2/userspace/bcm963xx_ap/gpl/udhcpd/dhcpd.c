/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"


/* globals */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;
static int signal_pipe[2];

/* Exit and cleanup */
static void exit_server(int retval)
{
	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* Signal handler */
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0) {
		LOG(LOG_ERR, "Could not send signal: %s", 
			strerror(errno));
	}
}

/* Fiji added Start, Silver, 2009/3/26, @STB */
#ifdef TELE2
int check_STB_vender(unsigned char *dhcpvendor)
{
    FILE *fp;
    char    command[128];
    char    line[64];
    int     match = 0;
    int     len;
    
    system("param show STB_name > /tmp/STB_name");
    fp = fopen("/tmp/STB_name", "r");
    if (fp!=NULL)
    {
   	    if ( fgets(line, 64, fp) > 0 ) 
      	{
      	    len = strlen("STB_name=");
      	    //printf("======%s=======\n", line);
      	    //sprintf(command, "param set kkk=%s", line+len);
      	    //system(command);    
      		if (strcmp(line+len, dhcpvendor)==0)
    		    match =1;
        }
        fclose(fp);
    }
    return match;
}
#endif
/* Fiji added End, Silver, 2009/3/26, @STB */

#ifdef COMBINED_BINARY	
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
	fd_set rfds;
	struct timeval tv;
	int server_socket = -1;
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
#if defined(TELE2)
	unsigned char   *dhcpvendor;
#endif	
#if defined(TI_ALICE) || defined(SingTel)
    unsigned char *server_id, *requested;
    unsigned char  *oldhostname, *hosttype,*dhcpvendor,newhostname[64];    /* Fiji added Start, Lewis, 2008/9/19, @Lan host identification */
#else
	unsigned char *server_id, *requested, *hostname;
#endif
    /* Fiji added Start, Silver, 2008/1/21, @TR111 */
#if (defined SUPPORT_TR111)
    unsigned char *tr111;
#endif
    /* Fiji added End, Silver, 2008/1/21, @TR111 */
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	int pid_fd;
	int max_sock;
	int sig;
	
	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);

	memset(&server_config, 0, sizeof(struct server_config_t));
	
	if (argc < 2)
		read_config(DHCPD_CONF_FILE);
	else read_config(argv[1]);

    /*fiji add start, water, 06/25/2008, @mlan on dvg834noud*/
    int whichlan = -1;
    if (strstr(argv[1], "udhcpd.conf") != NULL)
        whichlan = 0;
    else if (strstr(argv[1], "udhcpd_br1.conf") != NULL)
        whichlan = 1;
    else if (strstr(argv[1], "udhcpd_br2.conf") != NULL)
        whichlan = 2;
    else if (strstr(argv[1], "udhcpd_br3.conf") != NULL)
        whichlan = 3;
    /*fiji add end, water, 06/25/2008, @mlan on dvg834noud*/
    
    /* Fiji added Start, Silver, 2007/5/7, @TR111 */
#if (defined SUPPORT_TR111)
    initTR111();
    //acosNvramConfig_set ("TR111_num", "0");
    //acosNvramConfig_set ("TR111_tab", "");
    //acosNvramConfig_set ("TR111_notify_limit", "");
#endif /* SUPPORT_TR111 */
    /* Fiji added End, Silver, 2007/5/7, @TR111 */

    /*get reserved ip from configuration file*/
    /*fiji modified start, water, 06/25/2008, @mlan on dvg834noud*/
    //num_of_reservedIP = getReservedAddr(resrvMacAddr, resrvIpAddr);
    num_of_reservedIP = getReservedAddr(resrvMacAddr, resrvIpAddr, whichlan);
    /*fiji modified end, water, 06/25/2008, @mlan on dvg834noud*/

	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;
	
	leases = malloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	read_leases(server_config.lease_file);
#if defined(TI_ALICE) || defined(SingTel)
    create_que_id(); /* Fiji added Start, Lewis, 2008/9/19, @Lan host identification */
#endif
	if (read_interface(server_config.interface, &server_config.ifindex,
			   &server_config.server, server_config.arp) < 0)
		exit_server(1);

#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_server(1);
	}
	pidfile_write_release(pid_fd);
#endif


	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGTERM, signal_handler);

	timeout_end = time(0) + server_config.auto_time;
	while(1) { /* loop until universe collapses */

		if (server_socket < 0)
			if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface)) < 0) {
				LOG(LOG_ERR, "FATAL: couldn't create server socket, %s", strerror(errno));
				exit_server(0);
			}			

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		FD_SET(signal_pipe[0], &rfds);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - time(0);
			tv.tv_usec = 0;
		}
		if (!server_config.auto_time || tv.tv_sec > 0) {
			max_sock = server_socket > signal_pipe[0] ? server_socket : signal_pipe[0];
			retval = select(max_sock + 1, &rfds, NULL, NULL, 
					server_config.auto_time ? &tv : NULL);
		} else retval = 0; /* If we already timed out, fall through */

		if (retval == 0) {
			write_leases();
			timeout_end = time(0) + server_config.auto_time;
			continue;
		} else if (retval < 0 && errno != EINTR) {
			DEBUG(LOG_INFO, "error on select");
			continue;
		}
		
		if (FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0)
				continue; /* probably just EINTR */
			switch (sig) {
			case SIGUSR1:
				LOG(LOG_INFO, "Received a SIGUSR1");
				write_leases();
				/* why not just reset the timeout, eh */
				timeout_end = time(0) + server_config.auto_time;
				continue;
			case SIGTERM:
				LOG(LOG_INFO, "Received a SIGTERM");
				exit_server(0);
			}
		}

		if ((bytes = get_packet(&packet, server_socket)) < 0) { /* this waits for a packet - idle */
			if (bytes == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}

		if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
			continue;
		}
        
		/* ADDME: look for a static lease */
		lease = find_lease_by_chaddr(packet.chaddr);
		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			
            /* Fiji added Start, Silver, 2008/1/21, @TR111 */
#if (defined SUPPORT_TR111)
            tr111 = get_option(&packet, DHCP_TR111);
            if (tr111) getTR111Param(tr111);
#endif
            /* Fiji added End, Silver, 2008/1/21, @TR111 */

#if defined(SingTel)

            dhcpvendor = get_option(&packet, DHCP_VENDOR);

            if (dhcpvendor) 
            {
                bytes = dhcpvendor[-1];
                if (bytes >= 64)
                   bytes = 63;
                dhcpvendor[bytes]='\0';                   

                DEBUG(LOG_INFO, "\nserver_config.hostname (%s)",dhcpvendor);
            }
            else
                dhcpvendor = "DHCP Client";
#endif

#if defined(SingTel)
            if (sendOffer(&packet, dhcpvendor) < 0) 
#else
			if (sendOffer(&packet) < 0) 
#endif
			{
				LOG(LOG_ERR, "send OFFER failed");
			}
			break;			
 		case DHCPREQUEST:
        {
 		    /* foxcon added start by EricHuang, 03/01/2007 */
 		    unsigned char mac[6];
 		    u_int32_t r_addr;
 		    memcpy(mac, packet.chaddr, 6);
 		    /* foxcon added end by EricHuang, 03/01/2007 */
 		    
			DEBUG(LOG_INFO, "received REQUEST");

			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);
            /* Fiji added Start, Lewis, 2008/9/19, @Lan host identification */
#if defined(TI_ALICE)
            
            oldhostname = get_option(&packet, DHCP_HOST_NAME);
            hosttype = get_option(&packet, DHCP_HOST_TYPE);
            dhcpvendor = get_option(&packet, DHCP_VENDOR);

            if (oldhostname) {
                  bytes = oldhostname[-1];
                  if (bytes >= 64)
                     bytes = 63;
                     oldhostname[bytes] = '\0';
            } else
                  oldhostname = NULL;
            
            if (dhcpvendor) {
                bytes = dhcpvendor[-1];
                   if (bytes >= 64)
                   bytes = 63;
                dhcpvendor[bytes]='\0';                   
            } else
                    dhcpvendor = NULL;
            
            //printf("\nreceived REQUEST from %s vendorid ",oldhostname);
            check_host_name(oldhostname,newhostname,packet.chaddr);
            strcpy(server_config.hostname, newhostname);
            DEBUG(LOG_INFO, "\nserver_config.hostname %s",server_config.hostname);

#elif defined(SingTel)

            oldhostname = get_option(&packet, DHCP_HOST_NAME);
            hosttype = get_option(&packet, DHCP_HOST_TYPE);
            dhcpvendor = get_option(&packet, DHCP_VENDOR);
            
            if (oldhostname) {
                  bytes = oldhostname[-1];
                  if (bytes >= 64)
                     bytes = 63;
                     oldhostname[bytes] = '\0';
            } else
                  oldhostname = NULL;

            
            if (dhcpvendor) 
            {
                bytes = dhcpvendor[-1];
                if (bytes >= 64)
                   bytes = 63;
                dhcpvendor[bytes]='\0';                   

                DEBUG(LOG_INFO, "\nserver_config.hostname (%s)",dhcpvendor);
            } 
            else
                dhcpvendor = "";

            check_host_name(oldhostname,newhostname,packet.chaddr);
            //strcpy(server_config.hostname, newhostname);
            DEBUG(LOG_INFO, "\nserver_config.hostname %s",server_config.hostname);

#else
			hostname = get_option(&packet, DHCP_HOST_NAME);
#endif
            /* Fiji added end, Lewis, 2008/9/19, @Lan host identification */

            /* Fiji added Start, Silver, 2009/3/26, @STB */
#ifdef TELE2
            
            dhcpvendor = get_option(&packet, DHCP_VENDOR);

            if (dhcpvendor) 
            {
                bytes = dhcpvendor[-1];
                   if (bytes >= 64)
                   bytes = 63;
                dhcpvendor[bytes]='\0';                   
            
                //printf("\nreceived REQUEST from %s vendorid ",dhcpvendor);
                if (check_STB_vender(dhcpvendor))
                {
                    set_STB_reserved_ip(mac);
                }
                DEBUG(LOG_INFO, "\nserver_config.hostname %s",dhcpvendor);
            }
            //check_STB_vender(dhcpvendor);
#endif
            /* Fiji added End, Silver, 2009/3/26, @STB */
            /* Fiji added Start, Silver, 2008/1/21, @TR111 */
#if (defined SUPPORT_TR111)
            tr111 = get_option(&packet, DHCP_TR111);
            if (tr111) getTR111Param(tr111);
#endif
            /* Fiji added End, Silver, 2008/1/21, @TR111 */

			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);

            /* foxcon added start by EricHuang, 03/01/2007 */
            r_addr = find_reserved_ip(mac);
            if (r_addr) {
                if (requested_align)
                {
                    if ( requested_align!=htonl(r_addr))
                    {
                        sendNAK(&packet);
                        DEBUG(LOG_INFO, "prepare to send a reserved ip (0x%x, 0x%x)\n", requested_align, htonl(r_addr));
                        break;
                    }
                }
            }
            /* foxcon added end by EricHuang, 03/01/2007 */

			if (lease) { /*ADDME: or static lease */
				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested && 
					    requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr);
					}
				} else {
					if (requested) {
						/* INIT-REBOOT State */
						if (lease->yiaddr == requested_align)
							sendACK(&packet, lease->yiaddr);
						else sendNAK(&packet);
					} else {
						/* RENEWING or REBINDING State */
						if (lease->yiaddr == packet.ciaddr)
							sendACK(&packet, lease->yiaddr);
						else {
							/* don't know what to do!!!! */
							sendNAK(&packet);
						}
					}						
				}
                           
                /* Fiji added start, Lewis, 2008/9/19, @Lan host identification */
#if defined(TI_ALICE) || defined(SingTel)
                
                LOG(LOG_INFO,"rewrite leass table, newhostname: %s\n", newhostname);
                strcpy(lease->hostname, newhostname);
                if(0!=dhcpvendor)
                    strcpy(lease->vendorid, dhcpvendor);
                else
                    lease->vendorid[0]='\0';
                LOG(LOG_INFO,"rewrite leass table, hostname: %s\n", lease->hostname);

                if (hosttype)
                {
                           memcpy(&lease->hosttype, hosttype, 4);
                           LOG(LOG_INFO,"rewrite lease table, hosttype: %d\n", lease->hosttype);
                }
                else
                           lease->hosttype =0;

                LOG(LOG_INFO,"new_host_add %s\n",lease->vendorid);

                new_host_add(lease->yiaddr,lease->hosttype,lease->vendorid, newhostname );
                LOG(LOG_INFO,"new_host_add: \n");
#else
				if (hostname) {
					bytes = hostname[-1];
					if (bytes >= (int) sizeof(lease->hostname))
						bytes = sizeof(lease->hostname) - 1;
					strncpy(lease->hostname, hostname, bytes);
					lease->hostname[bytes] = '\0';
                    DEBUG(LOG_INFO,"rewrite leass table, hostname: %s\n", lease->hostname);
                    /* fiji wklin removed, 05/07/2007 */
                    /* write_leases(); */ /*Rewrite lease table into file.*/
				} else
					lease->hostname[0] = '\0';
                /* fiji wklin added, 05/07/2007 */
#endif
                /* Fiji added end, Lewis, 2008/9/19, @Lan host identification */
                write_leases(); /*Rewrite lease table into file.*/
			
			/* what to do if we have no record of the client */
			} else if (server_id) {
				/* SELECTING State */

			} else if (requested) {
				/* INIT-REBOOT State */
				if ((lease = find_lease_by_yiaddr(requested_align))) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
                        write_leases(); /*Rewrite lease table into file.*/
					/* make some contention for this address */
					} else sendNAK(&packet);
				} else if (requested_align < server_config.start || 
					   requested_align > server_config.end) {
					sendNAK(&packet);
				} else {
					sendNAK(&packet);
				}

			} else if (packet.ciaddr) {
				/* RENEWING or REBINDING State */
				sendNAK(&packet);
			}
			break;
		}
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
				lease->expires = time(0) + server_config.decline_time;
                write_leases(); /*Rewrite lease table into file.*/
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");

    /* Fiji added start , 08/08/2009 */
    /* by request, it needs to release all binding ... */	
#if defined(SingTel)
            //dhcp_releaseall();
#endif
    /* Fiji added end , 08/08/2009 */	
			
			if (lease) 
            {    
                lease->expires = time(0);
#if defined(TI_ALICE) || defined(SingTel)
                host_delete(lease->yiaddr);/* Fiji added start, Lewis, 2008/9/19, @Lan host identification */
#endif
                write_leases(); /*Rewrite lease table into file.*/
            }
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
	}

	return 0;
}

