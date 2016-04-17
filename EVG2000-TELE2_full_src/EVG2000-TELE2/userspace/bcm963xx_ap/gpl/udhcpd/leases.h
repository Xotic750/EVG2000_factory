/* leases.h */
#ifndef _LEASES_H
#define _LEASES_H


struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;	/* network order */
	u_int32_t expires;	/* host order */
	char hostname[64];
/* Foxconn added start, Lewis, 2008/9/19, @Lan host identification */
#if defined(TI_ALICE) || defined(SingTel)
    u_int32_t hosttype;
    char vendorid[64];
#endif
/* Foxconn added end, Lewis, 2008/9/19, @Lan host identification */
};

extern unsigned char blank_chaddr[];

void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr);
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease);
int lease_expired(struct dhcpOfferedAddr *lease);
struct dhcpOfferedAddr *oldest_expired_lease(void);
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr);
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr);
u_int32_t find_address(int check_expired);
u_int32_t find_address2(int check_expired, unsigned char *chaddr);
int check_ip(u_int32_t addr);


#endif