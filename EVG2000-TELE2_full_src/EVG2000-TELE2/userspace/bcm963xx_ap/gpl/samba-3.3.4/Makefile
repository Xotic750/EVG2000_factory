
all:
	$(MAKE) -C ./source


install:
	install -d $(INSTALL_DIR)/usr/local/
	install -d $(INSTALL_DIR)/usr/local/samba
	install -d $(INSTALL_DIR)/usr/local/samba/lib
	install -d $(INSTALL_DIR)/var/samba/
	install -d $(INSTALL_DIR)/var/samba/private
	install -d $(INSTALL_DIR)/etc
	install -m 755 ./data/group $(INSTALL_DIR)/etc
	install -m 755 ./data/lmhosts $(INSTALL_DIR)/usr/local/samba/lib
	install -m 755 ./source/bin/smb_pass  $(INSTALL_DIR)/usr/local/samba/
	install -m 755 ./source/bin/nmbd  $(INSTALL_DIR)/usr/local/samba/
	install -m 755 ./source/bin/smbd  $(INSTALL_DIR)/usr/local/samba/
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
	cd $(INSTALL_DIR)/etc && unlink passwd || pwd
	cd $(INSTALL_DIR)/etc && ln -sf ../tmp/samba/private/passwd passwd

clean:
	$(MAKE) -C ./source clean

