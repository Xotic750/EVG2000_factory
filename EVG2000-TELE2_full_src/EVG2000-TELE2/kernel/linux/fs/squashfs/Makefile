#
# Makefile for the linux squashfs routines.
#

obj-$(CONFIG_SQUASHFS) += squashfs.o
squashfs-y += inode.o
squashfs-y += squashfs2_0.o
# CONFIG_MIPS_BRCM Begin Broadcom changed code
squashfs-y += lzma/uncomp.o
squashfs-y += lzma/LzmaDecode.o
# CONFIG_MIPS_BRCM End Broadcom changed code

