###################################################################################################
# Common Export
CC      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)ar
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
CPP     = $(CC) -E
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB  = $(CROSS_COMPILE)ranlib
SIZE    = $(CROSS_COMPILE)size
PERL    = perl


INSTALL_BIN:=install -m0755
INSTALL_DIR:=install -d -m0755
INSTALL_DATA:=install -m0644
INSTALL_CONF:=install -m0600


export PATH:=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:$(TOOLCHAIN_BIN_DIR)

