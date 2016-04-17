all: .conf makesmb
.conf:
	cd source ; ./configure --without-python --with-included-popt --without-acl-support --without-spinlocks --without-profiling-data --without-syslog --without-nisplus-home --without-ldapsam --without-pam_smbpass --without-pam --without-smbmount --without-automount --without-expsam --without-krb5 --without-dce-dfs --without-vfs-afsacl --without-fake-kaserver --without-afs --without-smbwrapper --without-swatdir --without-fhs --disable-xmltest --disable-dmalloc --disable-krb5developer --disable-developer --disable-debug --without-shared-modules --without-readline --without-utmp --without-quotas --without-sys-quotas --without-ads --disable-cups --without-libsmbclient --without-smbmount --without-ldap --target=mipsel-linux --host=i686-linux --without-winbind --prefix=/usr/local/samba 
#	cd source ; ./configure --without-python --with-included-popt --without-acl-support --without-spinlocks --without-profiling-data --without-syslog --without-nisplus-home --without-ldapsam --without-pam_smbpass --without-pam --without-smbmount --without-automount --without-expsam --without-krb5 --without-dce-dfs --without-vfs-afsacl --without-fake-kaserver --without-afs --without-smbwrapper --without-swatdir --without-fhs --disable-xmltest --disable-dmalloc --disable-krb5developer --enable-developer --enable-debug --without-shared-modules --without-readline --without-utmp --without-quotas --without-sys-quotas --without-ads --disable-cups --without-libsmbclient --without-smbmount --without-ldap --target=mipsel-linux --host=i686-linux --without-winbind --prefix=/usr/local/samba
	touch .conf
makesmb:
#ifeq ($(PLATFORM_TYPE),EVG2000)
	cp config_EVG2000.h source/include/config.h
#endif
	cd source ; make

clean: .conf cleansmb
	
cleansmb: 
	cd source ; make clean	

distclean:
	rm -rf .conf
	cd source ; make distclean

install:
	install -d $(INSTALL_DIR)/usr/local/
	install -d $(INSTALL_DIR)/usr/local/samba
	install -d $(INSTALL_DIR)/usr/local/samba/lib
	install -d $(INSTALL_DIR)/var/samba/
	install -d $(INSTALL_DIR)/var/samba/private
	install -d $(INSTALL_DIR)/etc
	install -m 755 data/group $(INSTALL_DIR)/etc
	#install -m 755 data/smb.conf $(INSTALL_DIR)/usr/local/samba/lib
	install -m 755 data/lmhosts $(INSTALL_DIR)/usr/local/samba/lib
	install -m 755 source/bin/smb_pass  $(INSTALL_DIR)/usr/local/samba/
	install -m 755 source/bin/nmbd  $(INSTALL_DIR)/usr/local/samba/
	install -m 755 source/bin/smbd  $(INSTALL_DIR)/usr/local/samba/
	$(STRIP) $(INSTALL_DIR)/usr/local/samba/smbd
	$(STRIP) $(INSTALL_DIR)/usr/local/samba/nmbd
	$(STRIP) $(INSTALL_DIR)/usr/local/samba/smb_pass
	cd $(INSTALL_DIR)/usr/local/samba && unlink  private || pwd
	cd $(INSTALL_DIR)/usr/local/samba && unlink  var || pwd
	cd $(INSTALL_DIR)/usr/local/samba && unlink  lock || pwd
	cd $(INSTALL_DIR)/usr/local/samba && ln -sf ../../../var/samba/private private
	cd $(INSTALL_DIR)/usr/local/samba && ln -sf ../../../var var
	cd $(INSTALL_DIR)/usr/local/samba && ln -sf ../../../var/lock lock
	cd $(INSTALL_DIR)/usr/local/samba/lib && ln -sf ../../../../tmp/samba/private/smb.conf smb.conf
	#
	#	dummy install for smbd printer part	
	#
	#cp ../LPRng-3.8.32/printcap ./
	#install -m 755 printcap $(INSTALL_DIR)/etc
	cd $(INSTALL_DIR)/etc && unlink passwd || pwd
	#cd $(INSTALL_DIR)/etc && ln -sf ../var/samba/private/passwd passwd
	cd $(INSTALL_DIR)/etc && ln -sf ../tmp/samba/private/passwd passwd
