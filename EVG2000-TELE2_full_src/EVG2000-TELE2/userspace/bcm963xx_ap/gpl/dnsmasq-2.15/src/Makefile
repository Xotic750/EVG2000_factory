include ../../config.mk

# Uncomment this on Solaris.
#LIBS = -lsocket -lnsl

CFLAGS?= -O2

# NO IPv6 support at current stat. Must define, otherwise config.h will define HAVE_IPV6, 
# which may cause IPv4 dns query blocked.                 weal,April 28 
CFLAGS += -DNO_IPV6  

#OBJS = cache.o rfc1035.o util.o option.o forward.o isc.o \
#       network.o dnsmasq.o dhcp.o lease.o rfc2131.o 

OBJS = cache.o rfc1035.o util.o option.o forward.o \
       network.o dnsmasq.o 

.c.o: dnsmasq.h config.h 
	$(CC) $(CFLAGS) $(RPM_OPT_FLAGS) -Wall -W -c $*.c 

dnsmasq : $(OBJS) dnsmasq.h config.h
	$(CC) -o $@  $(OBJS) $(LIBS)



