all: .conf makelpr
.conf:
	cd LPRng-3.8.32 ; ./configure --target=mipsel-linux --host=i686-linux --disable-ssl --disable-nls --disable-kerberos --disable-mit_kerberos4 --disable-priv_ports --disable-klprpde --disable-tcpwrappers --with-lpd_conf_path=/etc/lpd.conf --with-lpd_perms_path=/etc/lpd.perms --with-printcap_path=/etc/printcap
	touch .conf
makelpr:
	cd LPRng-3.8.32 ; make

clean: .conf cleanlpr

cleanlpr:
	cd LPRng-3.8.32 ; make clean
	
distclean: .conf
	rm -rf .conf
	cd LPRng-3.8.32 ; make distclean
	
install:
	install -d $(INSTALL_DIR)/etc
	install -d $(INSTALL_DIR)/usr
	install -d $(INSTALL_DIR)/usr/bin
	install -m 755 LPRng-3.8.32/lpd.conf $(INSTALL_DIR)/etc
	install -m 755 LPRng-3.8.32/lpd.perms $(INSTALL_DIR)/etc
	install -m 755 hosts $(INSTALL_DIR)/etc
	install -m 755 host.conf $(INSTALL_DIR)/etc
	install -m 755 printcap $(INSTALL_DIR)/etc
	install -m 755 LPRng-3.8.32/src/lpr $(INSTALL_DIR)/usr/bin/
	install -m 755 LPRng-3.8.32/src/lpd $(INSTALL_DIR)/usr/bin/
	install -m 755 LPRng-3.8.32/src/lprm $(INSTALL_DIR)/usr/bin/
	install -m 755 LPRng-3.8.32/src/lpq $(INSTALL_DIR)/usr/bin/
	install -m 755 LPRng-3.8.32/src/lpc $(INSTALL_DIR)/usr/bin/
	$(STRIP) $(INSTALL_DIR)/usr/bin/lpr
	$(STRIP) $(INSTALL_DIR)/usr/bin/lprm
	$(STRIP) $(INSTALL_DIR)/usr/bin/lpq
	$(STRIP) $(INSTALL_DIR)/usr/bin/lpc
	$(STRIP) $(INSTALL_DIR)/usr/bin/lpd
