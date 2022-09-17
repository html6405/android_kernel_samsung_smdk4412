#!/system/xbin/bash
# Koffee's LATE startup script
# running when system already booted
# do not edit!
/sbin/busybox mount -o remount,rw /

# 1. Fix Doze helper permissions
/sbin/supolicy --live "allow kernel system_file file { execute_no_trans }"

# 2. Try to prefetch apps using readahead [2]
/system/bin/toybox readahead /system/lib/*.so
/system/bin/toybox readahead /data/dalvik-cache/arm/*.dex

# 3. FS Trim [2]
/sbin/busybox fstrim -v /data

# 4. Sdcard buffer tweaks [2]
/sbin/busybox echo 2048 > /sys/block/mmcblk0/bdi/read_ahead_kb
/sbin/busybox echo 1024 > /sys/block/mmcblk1/bdi/read_ahead_kb

# Clean up and fire up SELinux
/sbin/busybox echo 1 > /sys/module/koffee_late/parameters/hooked
/sbin/busybox rm /koffee-early.sh; /sbin/busybox rm /koffee-late.sh; /sbin/busybox mount -o remount,ro /libs; /sbin/busybox mount -o remount,ro /; /system/bin/toybox setenforce 1
exit 0