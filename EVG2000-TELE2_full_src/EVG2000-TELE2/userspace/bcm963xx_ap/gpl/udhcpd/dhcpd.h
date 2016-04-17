/* dhcpd.h */
#ifndef _DHCPD_H
#define _DHCPD_H

#include <netinet/ip.h>
#include <netinet/udp.h>

#include "libbb_udhcp.h"
#include "leases.h"

/************************************/
/* Defaults _you_ may want to tweak */
/************************************/

/* the period of time the client is allowed to use that address */
#define LEASE_TIME              (60*60*24*10) /* 10 days of seconds */

/* where to find the DHCP server configuration file */
#define DHCPD_CONF_FILE         "/etc/udhcpd.conf"

/*****************************************************************/
/* Do not modify below here unless you know what you are doing!! */
/*****************************************************************/

/* DHCP protocol -- see RFC 2131 */
#define SERVER_PORT		67
#define CLIENT_PORT		68

#define DHCP_MAGIC		0x63825363

/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d
#if defined(TI_ALICE) || defined(SingTel)
#define DHCP_HOST_TYPE    0x7d /* Foxconn added , Lewis, 2008/9/19, @Lan host identification */
#endif
/* Foxconn added Start, Silver, 2008/1/21, @TR111 */
#if (defined SUPPORT_TR111)
#define DHCP_TR111		0x7D
#endif
/* Foxconn added End, Silver, 2008/1/21, @TR111 */
/* Foxconn added Start, Silver, 2009/3/24, @OPT43 */
#if (defined TELE2)
#define DHCP_TR069_URL		0x2b
#endif
/* Foxconn added End, Silver, 2009/3/24, @OPT43 */



#define DHCP_END		0xFF


#define BOOTREQUEST		1
#define BOOTREPLY		2

#define ETH_10MB		1
#define ETH_10MB_LEN		6

#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8

#define BROADCAST_FLAG		0x8000

#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

/* miscellaneous defines */
#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
/* Foxconn added start, Lewis, 2008/9/19, @Lan host identification */
#ifdef TI_ALICE
#define STB_STRING    "stb" /* Foxconn added end, Lewis, 2008/9/19, @Lan host identification */
typedef enum {
    MANAGED_HOST=1,
    DATA_HOST,
    SIP_HOST,
    STB_HOST,
} dhcp_lan_host_type_t;

typedef enum {
    IPTV_OFF,
    IPTV_Natted,
    IPTV_Dyn_Portmapping,
    IPTV_Static_Portmapping,
} IT_IPTVService_type_t;
#endif
/* Foxconn added end, Lewis, 2008/9/19, @Lan host identification */

    /* Foxconn added start , 08/05/2009 */
#if defined(SingTel)
#define IP_DECT_HOST    1
#define STB_HOST        2
#define XBOX360_HOST    3
#define DATA_HOST       4
#endif
    /* Foxconn added end , 08/05/2009 */

#define MAX_TOKEN_SIZE    100
#define MAX_RESERVED_IP    64   /* Foxconn modified pling 10/04/2007, 30->64 */
#define MAX_RESERVED_MAC   64   /* Foxconn modified pling 10/04/2007, 30->64 */

struct option_set {
	unsigned char *data;
	struct option_set *next;
};

struct server_config_t {
	u_int32_t server;		/* Our IP, in network order */
	u_int32_t start;		/* Start address of leases, network order */
	u_int32_t end;			/* End of leases, network order */
	struct option_set *options;	/* List of DHCP options loaded from the config file */
	char *interface;		/* The name of the interface to use */
	int ifindex;			/* Index number of the interface to use */
	unsigned char arp[6];		/* Our arp address */
	unsigned long lease;		/* lease time in seconds (host order) */
	unsigned long max_leases; 	/* maximum number of leases (including reserved address) */
	char remaining; 		/* should the lease file be interpreted as lease time remaining, or
			 		 * as the time the lease expires */
	unsigned long auto_time; 	/* how long should udhcpd wait before writing a config file.
					 * if this is zero, it will only write one on SIGUSR1 */
	unsigned long decline_time; 	/* how long an address is reserved if a client returns a
				    	 * decline message */
	unsigned long conflict_time; 	/* how long an arp conflict offender is leased for */
	unsigned long offer_time; 	/* how long an offered address is reserved */
	unsigned long min_lease; 	/* minimum lease a client can request*/
	char *lease_file;
	char *pidfile;
	char *notify_file;		/* What to run whenever leases are written */
	u_int32_t siaddr;		/* next server bootp option */
	char *sname;			/* bootp server name */
	char *boot_file;		/* bootp boot file option */
/* Foxconn added start, Lewis, 2008/9/19, @Lan host identification */
#if defined(TI_ALICE)
    char hostname[64];     /* Host name that add to boot option */
    char *IPTV_service;/* IPTV service get from NVRAM */
#endif
/* Foxconn added end, Lewis, 2008/9/19, @Lan host identification */       
    /* pling added start 11/13/2008, for TR111 */
#if (defined SUPPORT_TR111)
    char oui[6+1];
    char serial_num[64+1];
    char product_class[64+1];
#endif
    /* pling added end 11/13/2008 */
    /* Foxconn added start , 08/05/2009 */
#if defined(SingTel)
    u_int32_t other_start;
    u_int32_t other_end;
    u_int32_t ip_dect_start;
    u_int32_t ip_dect_end;
    u_int32_t stb_start;
    u_int32_t stb_end;
    u_int32_t xbox_start;
    u_int32_t xbox_end;
#endif
    /* Foxconn added end , 08/05/2009 */
};

extern struct server_config_t server_config;
extern struct dhcpOfferedAddr *leases;
extern char resrvMacAddr[MAX_RESERVED_MAC][MAX_TOKEN_SIZE];
extern char resrvIpAddr[MAX_RESERVED_IP][MAX_TOKEN_SIZE];
extern int num_of_reservedIP;
/*foxconn modified start, water, 06/25/2008, @mlan on dvg834noud*/
//extern int getReservedAddr(char reservedMacAddr[][MAX_TOKEN_SIZE], char reservedIpAddr[][MAX_TOKEN_SIZE]);
extern int getReservedAddr(char reservedMacAddr[][MAX_TOKEN_SIZE], char reservedIpAddr[][MAX_TOKEN_SIZE], int whichlan);
/*foxconn modified end, water, 06/25/2008, @mlan on dvg834noud*/
extern int check_reserved_ip(u_int32_t req_ip, u_int8_t *chaddr);
extern u_int32_t find_reserved_ip(u_int8_t *chaddr);
#if (defined TELE2)
extern void set_STB_reserved_ip(u_int8_t *chaddr);
#endif
/* Foxconn added Start, Silver, 2007/5/7, @TR111 */
#if (defined SUPPORT_TR111)
extern int initTR111();
extern int getTR111Param(unsigned char *tr111);
#endif
/* Foxconn added End, Silver, 2007/5/7, @TR111 */

#endif
