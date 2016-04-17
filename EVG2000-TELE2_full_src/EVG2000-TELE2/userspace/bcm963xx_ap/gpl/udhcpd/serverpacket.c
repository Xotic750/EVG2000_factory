/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"

////
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp, fmt , ## args); \
		fclose(fp); \
	} \
} while (0)

#if defined(U12H154_TELE2)
#define STB_LISG        "/tmp/stb_list"
#define STB_LISG_TMP    "/tmp/stb_list_tmp"

void add_stb_list(char *stb_ip)
{
    FILE *fp=0;
    
    if ( (fp = fopen(STB_LISG, "a+")) != NULL )
    {
        char buf[16]="";
        
        while( fgets(buf, 16, fp) )
        {
            cprintf("List %s\n", buf);
            if (strstr(buf, stb_ip) )
            {
                cprintf("STB %s is already in the list\n", stb_ip);
                fclose(fp);
                return;
            }
            memset(buf, 0, 16);
        }
        cprintf("Add STB %s to list\n", stb_ip);
        fprintf(fp, "%s\n", stb_ip);
        fclose(fp);
    }
}

void del_stb_list(char *stb_ip)
{
    FILE *fp=0, *fp2=0;
    
    //cprintf("check STB lease %s\n", stb_ip);
    
    if ( (fp = fopen(STB_LISG, "r+")) != NULL )
    {
        char buf[16]="";
     
        if ( (fp2 = fopen(STB_LISG_TMP, "a+")) != NULL ) 
        {
            char cmd[64];
            while( fgets(buf, 16, fp) )
            {
                //cprintf("List %s\n", buf);
                if ( strstr(buf, stb_ip) || strlen(buf) < 6)
                {
                    cprintf("lease expired, remove STB %s from the list\n", stb_ip);
                    continue;
                }
                if ( strlen(buf) > 6 ) /* workaround */
                    fprintf(fp2, "%s", buf);
            }
            fclose(fp);
            fclose(fp2);
            sprintf(cmd, "rm -f %s", STB_LISG);
            system(cmd);
            sprintf(cmd, "cp %s %s", STB_LISG_TMP, STB_LISG);
            system(cmd);
            sprintf(cmd, "rm -f %s", STB_LISG_TMP);
            system(cmd);
        }
    }
}

#endif

/* Fiji added Start, Silver, 2007/5/7, @TR111 */
#if (defined SUPPORT_TR111)
#define TR111_code                      125
#define DeviceManufacurerOUI_code      	1
#define DeviceSerialNumber_code        	2
#define DeviceProductClass_code        	3

#define GatewayManufacurerOUI_code      4
#define GatewaySerialNumber_code        5
#define GatewayProductClass_code        6

unsigned char TR111Frame[200];

int initTR111(void)
{
    unsigned char GatewayManufacurerOUI[7] = "XXX";
    unsigned char GatewaySerialNumber[65] = "123456789";
    unsigned char GatewayProductClass[65] = "ADSL";
    unsigned int index=0;
    unsigned int length;

#if 0
    //fred start plugfest
    strcpy(GatewayManufacurerOUI,acosNvramConfig_get("manufacturer_OUI"));
    //strcpy(GatewaySerialNumber,acosNvramConfig_get("wan_hwaddr"));
    strcpy(GatewaySerialNumber,"00112233446C");
    strcpy(GatewayProductClass,acosNvramConfig_get("cpe_productclass"));
    //fred end plugfest
#endif
    strcpy(GatewayManufacurerOUI, server_config.oui);
    strcpy(GatewaySerialNumber, server_config.serial_num);
    strcpy(GatewayProductClass, server_config.product_class);

    TR111Frame[index] = TR111_code;
    index += 2;

    TR111Frame[index++] = 0x00;
    TR111Frame[index++] = 0x00;
    TR111Frame[index++] = 0x0d;
    TR111Frame[index++] = 0xe9;
    //TR111Frame[index++] = 0x17;
    index++;
 
    if( length = strlen(GatewayManufacurerOUI) )
    {
        TR111Frame[index++] = GatewayManufacurerOUI_code;
        TR111Frame[index++] = length;
        memcpy(&TR111Frame[index], GatewayManufacurerOUI, length);
        index += length;
    }
    if( length = strlen(GatewaySerialNumber) )
    {
        TR111Frame[index++] = GatewaySerialNumber_code;
        TR111Frame[index++] = length;
        memcpy(&TR111Frame[index], GatewaySerialNumber, length);
        index += length;
    }
    if( length = strlen(GatewayProductClass) )
    {
        TR111Frame[index++] = GatewayProductClass_code;
        TR111Frame[index++] = length;
        memcpy(&TR111Frame[index], GatewayProductClass, length);
        index += length;
    }
    TR111Frame[1] = (unsigned char)(index-2); /* total length of option */
    TR111Frame[6] = (unsigned char)(index-7);

    /* Clear these params */
    system("param set TR111_num=0");
    system("param set TR111_tab=\"\"");
    system("param set TR111_notify_limit=\"\"");

    return 1;
}

int getTR111Param(unsigned char *tr111)
{
	static unsigned int deviceNum=0;
	static unsigned char TR111Device[2000];
    unsigned char index=0;
    unsigned char index2=0;
    unsigned char TR111Temp[200];
	unsigned char length, subLen;
    char command[64];
    
    length = *(tr111-1);
    //length = *(tr111+4);
    index += 5;

    if(length != 0)
    {
    	if( tr111[index] == DeviceManufacurerOUI_code )
    	{
    		subLen=tr111[index+1];
    		index+=2;
    		memcpy(TR111Temp, tr111+index, subLen);
    		index+=subLen;
    		index2 += subLen;
    		TR111Temp[index2++]=':';
    	}
    	else
    		cprintf("Tr111 error\n");
    }
    if(index < length)
    {
    	if( tr111[index] == DeviceSerialNumber_code )
    	{
    		subLen=tr111[index+1];
    		index+=2;
    		memcpy(TR111Temp+index2, tr111+index, subLen);
    		index+=subLen;
    		index2 += subLen;
    		TR111Temp[index2++]=':';
    	}
    	else
    		cprintf("Tr111 error\n");
    }
    if(index < length)
    {
    	if( tr111[index] == DeviceProductClass_code )
    	{
    		subLen=tr111[index+1];
    		index+=2;
    		memcpy(TR111Temp+index2, tr111+index, subLen);
    		index+=subLen;
    		index2 += subLen;
    		TR111Temp[index2++]='\0';
    	}
    	else
    		cprintf("Tr111 error\n");
    }
cprintf("%s(%d) TR111Temp=(%s) deviceNum=%d\n", __FUNCTION__, __LINE__, TR111Temp, deviceNum);
    if(	deviceNum == 0 )
    {
    	memcpy( TR111Device, TR111Temp, index2);
    }
    else
    {
	    if( strstr( TR111Device, TR111Temp ) )
		    return 0;	
    	strcat( TR111Device, " " );
    	strcat( TR111Device, TR111Temp );
    }
    deviceNum++;

    /* pling modified start: Don't use acosNvramConfig_set */
    //acosNvramConfig_set("TR111_num", itoa(deviceNum));
    //acosNvramConfig_set("TR111_tab", TR111Device);
    sprintf(command, "param set TR111_num=%d", deviceNum);
    system(command);
    sprintf(command, "param set TR111_tab='%s'", TR111Device);
    system(command);
    /* pling modified end: Don't use acosNvramConfig_set */

	return 1;
}
#endif  /* SUPPORT_TR111 */
/* Fiji added End, Silver, 2007/5/7, @TR111 */
    

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	unsigned char *chaddr;
	u_int32_t ciaddr;
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config.server);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy(packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy(packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}


/* send a DHCP OFFER to a DHCP DISCOVER */
#if defined(SingTel)
int sendOffer(struct dhcpMessage *oldpacket, char *vendorid)
#else
int sendOffer(struct dhcpMessage *oldpacket)
#endif
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	unsigned char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;
    unsigned char mac[6];
	u_int32_t reserved_ip;

    memcpy(mac, oldpacket->chaddr, 6);
    
	init_packet(&packet, oldpacket, DHCPOFFER);
	
	/* ADDME: if static, short circuit */
	/* the client is in our lease/offered table */
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr)) &&
        
        /* Make sure the IP is not already used on network */
        !check_ip(lease->yiaddr)) {

		if (!lease_expired(lease)) 
			lease_time_align = lease->expires - time(0);
		packet.yiaddr = lease->yiaddr;
		
    /* Find a reserved ip for this MAC */
	} else if ( (reserved_ip = find_reserved_ip(mac)) != 0) {
		packet.yiaddr = htonl(reserved_ip);
        
	/* Or the client has a requested ip */
	} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

		/* Don't look here (ugly hackish thing to do) */
		memcpy(&req_align, req, 4) &&

        /* check if the requested ip has been reserved */
        check_reserved_ip(req_align, mac) &&
           
		/* and the ip is in the lease range */
		ntohl(req_align) >= ntohl(server_config.start) &&
		ntohl(req_align) <= ntohl(server_config.end) &&
		   
        /* Check that this request ip is not on network */
        !check_ip(ntohl(req_align)) &&
           
		/* and its not already taken/offered */ /* ADDME: check that its not a static lease */
		((!(lease = find_lease_by_yiaddr(req_align)) ||
		   
		/* or its taken, but expired */ /* ADDME: or maybe in here */
		lease_expired(lease)))) {
		    packet.yiaddr = req_align; 

	/* otherwise, find a free IP */ /*ADDME: is it a static lease? */
	} else {

#if defined(SingTel)	    
	    DEBUG(LOG_INFO, "\n>>> %s(%d) server_config.vendor_name (%s)", __FUNCTION__, __LINE__, server_config.vendor_name);
        packet.yiaddr = singtel_find_address(0, mac, vendorid);
#else
		packet.yiaddr = find_address2(0, mac);
#endif	
		/* try for an expired lease */
		if (!packet.yiaddr) packet.yiaddr = find_address2(1, mac);
	}
	
	if(!packet.yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		return -1;
	}
	
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}		

	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
	}

	/* Make sure we aren't just using the lease time from the previous offer */
	if (lease_time_align < server_config.min_lease) 
		lease_time_align = server_config.lease;
	/* ADDME: end of short circuit */		
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);
	
	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0);
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
	
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1);
}

#if defined(U12L060) || defined(U12L090) 
#include <fcntl.h> /* open */
#include <sys/ioctl.h> /* ioctl */

#define MAJOR_NUM 100
#define IOCTL_AG_LOG_POINT_GET              _IOR(MAJOR_NUM, 98, char *) 

static int agApiLogPointGet(int *pLogPoint)
{
    int ret_val, file_desc;

    file_desc = open("/dev/acos_nat_cli", O_RDWR);
	if (file_desc < 0) 
	{
		printf("Can't open device file: /dev/acos_nat_cli\n");
		return 0;
	}
    
	ret_val = ioctl(file_desc,IOCTL_AG_LOG_POINT_GET, pLogPoint);
	if (ret_val < 0)
	{
		printf("[%s]:ioctl_get_msg failed:%d\n",__FUNCTION__, ret_val);
		return 0;
	}

       close(file_desc); 
       
       return 1;
}
#endif

#if defined(U12H154_TELE2)
#include <fcntl.h> /* open */
#include <sys/ioctl.h> /* ioctl */

#define MAJOR_NUM 100
#define AG_MAX_ARG_CNT        32
#define AG_MAX_ARG_LEN    100 
#define IOCTL_AG_RULE_ADD2  _IOR(MAJOR_NUM, 40, char *) /*add a rule and return rule id*/

typedef struct agIoctlParamPack
{ 
    int         argc;
    char*       argv[AG_MAX_ARG_CNT];
} T_agIoctlParam;   

static int agApi_natRuleAdd2(char *szLanIP)
{
	int ret_val=0, file_desc, i;
	T_agIoctlParam	ioctlParam;
	
	file_desc = open("/dev/acos_nat_cli", O_RDWR);
	if (file_desc < 0) 
	{
		return 0;
	}
	
	memset(&ioctlParam,0,sizeof(T_agIoctlParam));
	
	ioctlParam.argc = 6;
    ioctlParam.argv[0] = "ruleadd";
    ioctlParam.argv[1] = "8080";
    ioctlParam.argv[2] = "8080";
    ioctlParam.argv[3] = szLanIP;
    ioctlParam.argv[4] = "8080";
    ioctlParam.argv[5] = "tcp";
	
	ret_val = ioctl(file_desc,IOCTL_AG_RULE_ADD2,&ioctlParam);

	close(file_desc);
	/* Fiji modify start, Vanessa Kuo, 12/19/2006, @R2_sec1.8 */
	if (ret_val <= -1)
		return ioctlParam.argc;
	else
    return ret_val;
	/* Fiji modify end, Vanessa Kuo, 12/19/2006, @R2_sec1.8 */
}
#endif

int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	unsigned char *lease_time;
	
	
	FILE *fp;
	unsigned char *client_mac, *client_ip;
	char logBuffer[96];
	
	
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;
	int logPoint = 0;

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
	
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}
/* Fiji added start, Lewis, 2008/9/19, @Lan host identification */
#ifdef TI_ALICE
    add_hostname_option(packet.options, server_config.hostname);
#endif
/* Fiji added end, Lewis, 2008/9/19, @Lan host identification */
/* Fiji added Start, Silver, 2007/5/7, @TR111 */
#if (defined SUPPORT_TR111)
    add_option_string(packet.options, TR111Frame);
#endif
/* Fiji added End, Silver, 2007/5/7, @TR111 */

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

	if (send_packet(&packet, 0) < 0) 
		return -1;

	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);
	

#if defined(U12H154_TELE2)
    {
        char *dhcpvendor;
        dhcpvendor = get_option(oldpacket, DHCP_VENDOR);

        if (dhcpvendor) 
        {
            char lan[24]="";
            int bytes = dhcpvendor[-1];
            if (bytes >= 64)
               bytes = 63;
            dhcpvendor[bytes]='\0';
            client_ip = (unsigned char *)&packet.yiaddr;                  
	        sprintf(lan, "%d.%d.%d.%d",*client_ip, *(client_ip+1), *(client_ip+2), *(client_ip+3));

            if ( strcmp(dhcpvendor, "Telenor_VIP2853") == 0 )
            {
                cprintf("Add rule map.\n");
                agApi_natRuleAdd2(lan); 
                add_stb_list(lan); /* add STB to a file */
            }
            cprintf("\n dhcpvendor (%s) lan=(%s)",dhcpvendor, lan);
        } 
    }
#endif


	client_mac = (unsigned char *)packet.chaddr;
	client_ip = (unsigned char *)&packet.yiaddr;
	sprintf(logBuffer, "[DHCP IP: (%d.%d.%d.%d)] to MAC address %02X:%02X:%02X:%02X:%02X:%02X,",
	                                          *client_ip, *(client_ip+1), *(client_ip+2), *(client_ip+3),
	                                          *client_mac, *(client_mac+1), *(client_mac+2), *(client_mac+3), *(client_mac+4), *(client_mac+5));

#if defined(U12L060) || defined(U12L090) 
    if (agApiLogPointGet(&logPoint) != 0)
    {
        if (logPoint & 0x04) //Include in Log: Router operation(start up, get time etc)
        {
        	if ((fp = fopen("/dev/aglog", "r+")) != NULL)
            {
                fwrite(logBuffer, sizeof(char), strlen(logBuffer)+1, fp);
                fclose(fp);
            }
        }
    }
#else
	if ((fp = fopen("/dev/aglog", "r+")) != NULL)
    {
        fwrite(logBuffer, sizeof(char), strlen(logBuffer)+1, fp);
        fclose(fp);
    }
#endif	
	

	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK);
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

/* Fiji added Start, Silver, 2007/5/7, @TR111 */
#if (defined SUPPORT_TR111)
    add_option_string(packet.options, TR111Frame);
#endif
/* Fiji added End, Silver, 2007/5/7, @TR111 */

	add_bootp_options(&packet);

	return send_packet(&packet, 0);
}



