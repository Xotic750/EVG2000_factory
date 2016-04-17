#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dproxy.h"
#include "conf.h"
#include <cms_params.h>

extern int dns_sock;
static char primary_ns[16];
static char secondary_ns[16];
static struct sockaddr_in probe_addr;
static time_t probe_next_time;
static int probe_tried;
static time_t probe_timeout;
static uint16_t probe_id;
static char probe_pkt[36] =
	{0x0, 0x0, 0x1, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	 0x1, 'a', 0xc, 'r', 'o', 'o', 't', '-', 's', 'e', 'r', 'v',
	 'e', 'r', 's', 0x3, 'n', 'e', 't', 0x0, 0x0, 0x1, 0x0, 0x1, };

/* Load name servers' addresses from /var/fyi/sys/dns */
static int load_brcm_name_servers(void)
{
	FILE * fp;
	char line[256];
	char addr[16];

	if((fp = fopen(STATIC_DNS_FILE_DEFAULT, "r")) == NULL)
		return 0;
	if(fgets(line, sizeof(line), fp)) {
		line[sizeof(line)-1] = 0;
		if (sscanf(line, "%*s %15s", addr) == 1)
			strcpy(primary_ns, addr);
	}
	if(fgets(line, sizeof(line), fp)) {
		line[sizeof(line)-1] = 0;
		if (sscanf(line, "%*s %15s", addr) == 1)
			/* No check for duplicate address */
			strcpy(secondary_ns, addr);
	}
	fclose(fp);

	debug("dproxy: load static dns1 %s, dns2 %s from %s\n",
	      primary_ns, secondary_ns, STATIC_DNS_FILE_DEFAULT);

	if (primary_ns[0] == 0 && secondary_ns[0] == 0)
		return 0;
	return 1;
}

/* Initialization before probing */
int dns_probe_init(void)
{
	int ret;
	
	/* Try to read name servers from /var/fyi/sys/dns */
	if (!(ret = load_brcm_name_servers()))
			/* If there is no name server configured,
			 * use the default */
			strcpy(primary_ns, config.name_server);

	/* Set primary server as the probing address */
	memset(&probe_addr, 0, sizeof(probe_addr));
	probe_addr.sin_family = AF_INET;
	inet_aton(primary_ns, &probe_addr.sin_addr);
	probe_addr.sin_port = ntohs(PORT);
	
	/* Initialize request id */
	srandom(time(NULL));
	probe_id = (uint16_t)random();
	return ret;
}

/* Send, resend probing request, check timeout, and switch name servers */
void dns_probe(void)
{
	time_t now = time(NULL);

	if (probe_tried) { /* Probing */
		if (now >= probe_timeout) { /* Timed out */
			if (probe_tried >= DNS_PROBE_MAX_TRY) {
				/* Probe failed */
				debug("dproxy: probing failed\n");
				if (secondary_ns[0] &&
				    strcmp(config.name_server, secondary_ns)) {
					printf("Primary DNS server Is Down... "
					       "Switching To Secondary DNS "
					       "server \n");
					strcpy(config.name_server,
					       secondary_ns);
				}
				probe_tried = 0;
				probe_next_time = now + DNS_PROBE_INTERVAL;
			} else { /* Retry */
				sendto(dns_sock, probe_pkt, sizeof(probe_pkt),
				       0, (struct sockaddr*)&probe_addr,
				       sizeof(probe_addr));
				probe_timeout = time(NULL) + DNS_PROBE_TIMEOUT;
				probe_tried++;
			}
		}
	} else if (now >= probe_next_time) { /* Time to probe */
		*((uint16_t*)probe_pkt) = htons(++probe_id);
		sendto(dns_sock, probe_pkt, sizeof(probe_pkt), 0,
		       (struct sockaddr*)&probe_addr, sizeof(probe_addr));
		probe_tried = 1;
		probe_timeout = time(NULL) + DNS_PROBE_TIMEOUT;
	}
}

/* Activate primary server */
int dns_probe_activate(uint32_t name_server)
{
	static int first_time = 1;

	if (name_server != probe_addr.sin_addr.s_addr)
		return 0;
	probe_tried = 0;
	probe_next_time = time(NULL) + DNS_PROBE_INTERVAL;
	if (strcmp(config.name_server, primary_ns) == 0)
		return 1;

	if (first_time)
		first_time = 0;
	else
		printf("switching back to primary dns server\n");

	strcpy(config.name_server, primary_ns);
	return 1;
}

/* Activate name server if it's the response for probing request */
int dns_probe_response(dns_request_t *m)
{
	if (m->message.header.flags.flags & 0x8000 &&
	    m->message.header.id != probe_id)
		return 0;
	return dns_probe_activate(m->src_addr.s_addr);
}

