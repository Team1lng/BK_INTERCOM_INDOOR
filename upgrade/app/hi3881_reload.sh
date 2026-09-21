#! /bin/sh



driver_reload(){
    killall wpa_supplicant
    ps | pgrep -f 'udhcpc.*wlan0.*' | xargs kill

    rmmod hi3881
    rmmod ak_mci
    rmmod cfg80211

    sleep 1

    modprobe cfg80211.ko
    insmod /usr/modules/ak_mci.ko
    insmod /usr/modules/hi3881.ko

    sleep 1

    wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B

    sleep 1

    udhcpc -i wlan0
}

# 需要放在后台运行, 防止客户端调用时造成堵塞
driver_reload &



