#ifndef _SERVERPACKET_H
#define _SERVERPACKET_H

#if defined(SingTel)
int sendOffer(struct dhcpMessage *oldpacket, char *vendorid);
#else
int sendOffer(struct dhcpMessage *oldpacket);
#endif
int sendNAK(struct dhcpMessage *oldpacket);
int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr);
int send_inform(struct dhcpMessage *oldpacket);


#endif
