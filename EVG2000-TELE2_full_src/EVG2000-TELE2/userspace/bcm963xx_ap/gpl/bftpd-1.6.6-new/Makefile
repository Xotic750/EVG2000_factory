# Generated automatically from Makefile.in by configure.

# pling added 04/27/2009
include ../config.mk
include ../config.in

VERSION=1.6.6
CFLAGS += -O2 -DHAVE_CONFIG_H -Wall -I. -DVERSION=\"$(VERSION)\" -D_LARGEFILE_SOURCE -DQUICK_FIX_ISSUES -ULEAN
all: bftpd
LIBS= -lcrypt

HEADERS=bftpdutmp.h commands.h commands_admin.h cwd.h dirlist.h list.h login.h logging.h main.h mystring.h options.h targzip.h mypaths.h
OBJS=bftpdutmp.o commands.o commands_admin.o cwd.o dirlist.o list.o login.o logging.o main.o mystring.o options.o
SRCS=bftpdutmp.c commands.c commands_admin.c cwd.c dirlist.c list.c login.c logging.c main.c mystring.c options.c

LDFLAGS=-Wl,-allow-shlib-undefined

bftpd: $(OBJS)
	./mksources $(DIRPAX)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o bftpd

install:
	install -m 755 bftpd $(INSTALL_DIR)/usr/sbin
	#$(STRIP) $(INSTALL_DIR)/bin/bftpd
	#cp -af bftpd.conf $(INSTALL_DIR)/etc

clean:
	-rm -f bftpd $(OBJS) mksources.finished config.cache
	rm -f $(INSTALL_DIR)/bin/bftpd
	#rm -f $(INSTALL_DIR)/etc/bftpd.conf

