cmd_/opt/FriendlyARM/toolschain/4.5.1/arm-none-linux-gnueabi/sys-root/usr/include/linux/can/.check := for f in bcm.h error.h netlink.h raw.h ; do echo "/opt/FriendlyARM/toolschain/4.5.1/arm-none-linux-gnueabi/sys-root/usr/include/linux/can/$${f}"; done | xargs perl /work/toolchain/build/src/linux-2.6.35.4/scripts/headers_check.pl /opt/FriendlyARM/toolschain/4.5.1/arm-none-linux-gnueabi/sys-root/usr/include arm; touch /opt/FriendlyARM/toolschain/4.5.1/arm-none-linux-gnueabi/sys-root/usr/include/linux/can/.check