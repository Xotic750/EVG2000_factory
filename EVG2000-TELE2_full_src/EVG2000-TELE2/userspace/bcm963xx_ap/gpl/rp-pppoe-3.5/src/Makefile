#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux user-space PPPoE client.
#
# Copyright (C) 2000 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# LIC: GPL
#
# $Id: Makefile.in,v 1.84 2002/07/05 19:32:50 dfs Exp $
#***********************************************************************
include ../../config.in
include ../../config.mk 
#Foxconn add Zz Shan 12/28/2007

# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
VERSION=3.5

# PPPoE relay -- currently only supported on Linux
#PPPOE_RELAY=pppoe-relay
CFLAGS= -g -O2 -Wall -Wstrict-prototypes -ansi -pedantic -Ilibevent

TARGETS=pppoe-relay

all: $(TARGETS)
	@echo ""
	@echo "Type 'make install' as root to install the software."

pppoe-relay: relay.o if.o debug.o common.o
	$(CC) -o pppoe-relay relay.o if.o debug.o common.o

pppoe.o: pppoe.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe.o pppoe.c

discovery.o: discovery.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o discovery.o discovery.c

ppp.o: ppp.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o ppp.o ppp.c

md5.o: md5.c md5.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o md5.o md5.c

pppoe-server.o: pppoe-server.c pppoe.h 
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe-server.o pppoe-server.c

pppoe-sniff.o: pppoe-sniff.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe-sniff.o pppoe-sniff.c

if.o: if.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o if.o if.c

libevent/libevent.a:
	cd libevent && $(MAKE)

common.o: common.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o common.o common.c

debug.o: debug.c pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o debug.o debug.c

relay.o: relay.c relay.h pppoe.h
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o relay.o relay.c

install: all
	install -d $(INSTALL_DIR)/usr/sbin
	install pppoe-relay $(INSTALL_DIR)/usr/sbin
	$(STRIP) $(INSTALL_DIR)/usr/sbin/pppoe-relay

clean:
	rm -f *.o *.a pppoe-relay $(INSTALL_DIR)/usr/sbin/pppoe-relay

.PHONY: clean
