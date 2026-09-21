# 当前文件所在目录
LOCAL_PATH := $(call my-dir)

#---------------------------------------
# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
LOCAL_MODULE := $(notdir $(LOCAL_PATH))

# 申明模块产物为SDK
LOCAL_MODULE_TAGS :=system sdk adapter

TKL_SRC_PATH := $(LOCAL_PATH)/src
TKL_INCLUDE_PATH := $(LOCAL_PATH)/include

# 模块对外头文件（只能是目录）
# 加载至CFLAGS中提供给其他组件使用；打包进SDK产物中；
LOCAL_TUYA_SDK_INC := $(TKL_INCLUDE_PATH)/adc \
	$(TKL_INCLUDE_PATH)/bluetooth \
	$(TKL_INCLUDE_PATH)/cellular_catx \
	$(TKL_INCLUDE_PATH)/dac \
	$(TKL_INCLUDE_PATH)/display \
	$(TKL_INCLUDE_PATH)/flash \
	$(TKL_INCLUDE_PATH)/gpio \
	$(TKL_INCLUDE_PATH)/hci \
	$(TKL_INCLUDE_PATH)/i2c \
	$(TKL_INCLUDE_PATH)/i2s \
	$(TKL_INCLUDE_PATH)/init \
	$(TKL_INCLUDE_PATH)/media \
	$(TKL_INCLUDE_PATH)/motor \
	$(TKL_INCLUDE_PATH)/nb \
	$(TKL_INCLUDE_PATH)/pinmux \
	$(TKL_INCLUDE_PATH)/pm \
	$(TKL_INCLUDE_PATH)/pwm \
	$(TKL_INCLUDE_PATH)/register \
	$(TKL_INCLUDE_PATH)/rtc \
	$(TKL_INCLUDE_PATH)/security \
	$(TKL_INCLUDE_PATH)/spi \
	$(TKL_INCLUDE_PATH)/storage \
	$(TKL_INCLUDE_PATH)/system \
	$(TKL_INCLUDE_PATH)/timer \
	$(TKL_INCLUDE_PATH)/uart \
	$(TKL_INCLUDE_PATH)/utilities/include \
	$(TKL_INCLUDE_PATH)/wakeup \
	$(TKL_INCLUDE_PATH)/watchdog \
	$(TKL_INCLUDE_PATH)/wifi \
	$(TKL_INCLUDE_PATH)/wired

# include目录下存在一些tuya提供的组件源文件，请勿修改
LOCAL_SRC_FILES += $(shell find $(TKL_INCLUDE_PATH)/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")

# 模块对外CFLAGS：其他组件编译时可感知到
LOCAL_TUYA_SDK_CFLAGS :=

# 全局变量赋值
TUYA_SDK_CFLAGS += $(LOCAL_TUYA_SDK_CFLAGS)  # 此行勿修改

LOCAL_CFLAGS := -Os -fPIC -Wall 

#LOCAL_CFLAGS += -I$(LOCAL_PATH)/../../scripts/error_code/release

#tkl_adc
ifeq ($(CONFIG_ENABLE_TKL_ADC),y)
ADC_DEV_INDEX:=$(call qstrip,$(CONFIG_TY_TKL_ADC_INDEX))
ADC_CHANS_PER_DEV:=$(call qstrip,$(CONFIG_TY_TKL_ADC_CHANS_PER_DEV))
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_adc/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")

LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/tkl_adc/include -DADC_DEV_INDEX=$(ADC_DEV_INDEX) -DADC_CHANS_PER_DEV=$(ADC_CHANS_PER_DEV)
endif

#tkl_wired 
ifeq ($(CONFIG_ENABLE_TKL_WIRED),y)
MY_LOCAL_CFLAGS := -DSSD20X

LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_wired/src/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_CFLAGS += $(MY_LOCAL_CFLAGS)
LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/tkl_wired/include
endif

#tkl_system 
ifeq ($(CONFIG_ENABLE_TKL_SYSTEM),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_system/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/tkl_system/include
LOCAL_CFLAGS += -DCREATE_RECURSION_MUTEX

ifeq ($(CONFIG_ENABLE_GET_FREE_HEAPP),y)
LOCAL_CFLAGS += -DENABLE_GET_FREE_HEAP
endif

ifeq ($(CONFIG_ENABLE_PRIORITY_SCHED),y)
LOCAL_CFLAGS += -DENABLE_PRIORITY_SCHED
endif

ifeq ($(CONFIG_ENABLE_LOG_OUTPUT_FORMAT),y)
LOCAL_CFLAGS += -DENABLE_LOG_OUTPUT_FORMAT
endif

endif

#tkl_wifi

ifeq ($(CONFIG_ENABLE_TKL_WIFI),y)

ifeq ($(CONFIG_ENABLE_HOSTAPD),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_wifi/src/func/tkl_wifi_hostap.c
endif

LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_wifi/src/func/tkl_gen_net.c
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_wifi/src/func/tkl_net.c
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_wifi/src/func/tkl_wifi.c
LOCAL_SRC_FILES += $(wildcard $(TKL_SRC_PATH)/tkl_wifi/src/func/wpa_s/*)

#LOCAL_SRC_FILES += $(wildcard $(TKL_SRC_PATH)/tkl_wifi/src/func/*.c)

LOCAL_SRC_FILES += $(wildcard $(TKL_SRC_PATH)/tkl_wifi/src/utils/sys_cmd.c)
LOCAL_SRC_FILES += $(wildcard $(TKL_SRC_PATH)/tkl_wifi/src/func/wpa_s/*.c)
LOCAL_SRC_FILES += $(wildcard $(TKL_SRC_PATH)/tkl_wifi/src/utils/iwlib/*.c)
LOCAL_SRC_FILES += $(wildcard $(TKL_SRC_PATH)/tkl_wifi/src/utils/wpa_cli/*.c)

LOCAL_CFLAGS +=  -I$(TKL_SRC_PATH)/tkl_wifi/include -I$(TKL_SRC_PATH)/tkl_wifi/src/utils/log -I$(TKL_SRC_PATH)/tkl_wifi/src/utils/wpa_cli -I$(TKL_SRC_PATH)/tkl_wifi/src/utils/iwlib -I$(TKL_SRC_PATH)/tkl_wifi/src/func/wpa_s

LOCAL_CFLAGS += -DWLAN_DEV=\"$(CONFIG_TKL_WIFI_MAIN_DEV)\"
LOCAL_CFLAGS += -DWLAN_AP=\"$(CONFIG_TKL_WIFI_CONCUR_DEV)\"

ifeq ($(CONFIG_TKL_WIFI_EXTERNAL_UDHCPC),y)
#use external udhcpc
$(warning echo external udhcpc)
else
$(warning echo own udhcpc)
LOCAL_CFLAGS += -DOWN_UDHCPC
endif

ifeq ($(CONFIG_TKL_WIFI_CFG_PATH_TMP),y)
LOCAL_CFLAGS += -DCFG_PATH_TMP
endif

ifeq ($(CONFIG_TKL_WIFI_INFACE_IS_NL80211),y)
LOCAL_CFLAGS += -DNL80211
else
LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0
LOCAL_CFLAGS += -DHAVE_CONFIG_H  -D_U_="__attribute__((unused))" -g -O2
LOCAL_SRC_FILES  +=  \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/pcap-linux.c         \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/pcap-usb-linux.c     \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/fad-getad.c          \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/pcap.c               \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/inet.c               \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/gencode.c            \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/optimize.c           \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/nametoaddr.c         \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/etherent.c           \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/savefile.c           \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/sf-pcap.c            \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/sf-pcap-ng.c         \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/pcap-common.c        \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/scanner.c            \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/grammar.c            \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/bpf_image.c          \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/bpf_dump.c           \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/bpf_filter.c         \
$(TKL_SRC_PATH)/tkl_wifi/src/utils/libpcap-1.4.0/version.c 
endif
endif

#tkl_storage
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_storage/src/df.c $(TKL_SRC_PATH)/tkl_storage/src/fdisk.c $(TKL_SRC_PATH)/tkl_storage/src/tkl_storage_api.c

#sys_log

ifeq ($(CONFIG_ENABLE_SYS_LOG),y)

LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/sys_log/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")

ifeq ($(CONFIG_TY_SYS_LOG_JOURNALD),y)
LOCAL_CFLAGS += -DCONFIG_TY_SYS_LOG_JOURNALD
endif

ifeq ($(CONFIG_TY_SYS_LOG_LEVEL_SAVE),y)
LOCAL_CFLAGS += -DCONFIG_TY_SYS_LOG_LEVEL_SAVE
endif

ifeq ($(CONFIG_TY_SYS_LOG_FILTER),y)
LOCAL_CFLAGS += -DCONFIG_TY_SYS_LOG_FILTER
endif

LOCAL_TY_SDK_INC += $(TKL_SRC_PATH)/sys_log/include
endif

#tkl_utils
ifeq ($(CONFIG_ENABLE_TKL_UTILS),y)

ifeq ($(CONFIG_TY_TKL_CMD),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_utils/src/cmd/tkl_cmd.c
endif

ifeq ($(CONFIG_TY_TKL_FILE),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_utils/src/file/tkl_file.c
endif

ifeq ($(CONFIG_TY_TKL_REBOOT),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_utils/src/reboot/tkl_reboot.c
endif

ifeq ($(CONFIG_TY_TKL_TAR),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_utils/src/tar/tkl_tar.c
endif

ifeq ($(CONFIG_TY_TKL_PASSWD),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_utils/src/passwd/tkl_passwd.c
LOCAL_LDFLAGS += -lcrypt
endif
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_utils/include
endif

#tkl_network_config
ifeq ($(CONFIG_ENABLE_TKL_NETWORK_CONFIG),y) 
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_network_config/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_network_config/include
endif

ifeq ($(CONFIG_ENABLE_TKL_IMU),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_imu/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_imu/include
endif

ifeq ($(CONFIG_ENABLE_TKL_IR),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_ir/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_CFLAGS += -DTY_TKL_IR_DEV_PATTERN=\"$(CONFIG_TY_TKL_IR_DEV_PATTERN)\"
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_ir/include
endif

ifeq ($(CONFIG_ENABLE_TKL_MOTOR),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_motor/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_motor/include
endif

ifeq ($(CONFIG_ENABLE_TKL_SENSOR),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_sensor/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_sensor/include
endif

ifeq ($(CONFIG_ENABLE_TKL_INPUT_EVENT),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_input_event/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_input_event/include
endif

#tkl_bus
ifeq ($(CONFIG_ENABLE_TKL_BUS),y)
ifeq ($(CONFIG_TY_TKL_UART),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_bus/src/uart/tkl_uart.c 
ifeq ($(TY_TKL_SYS_UART_PATTERN_SET),y)
LOCAL_CFLAGS += -DTKL_SYS_UART_PATTTERN=$(TY_TKL_SYS_UART_PATTERN)
endif
endif

ifeq ($(CONFIG_TY_TKL_PWM),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_bus/src/pwm/tkl_pwm.c 
endif

ifeq ($(CONFIG_TY_TKL_SPI),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_bus/src/spi/tkl_spi.c 
endif

ifeq ($(CONFIG_TY_TKL_I2C),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_bus/src/i2c/tkl_i2c.c 
endif

endif

#tkl_display
ifeq ($(CONFIG_ENABLE_TKL_DISPAY),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_display/src/tkl_display_api.c  \
                   $(TKL_SRC_PATH)/tkl_display/src/tkl_display_backlight.c

ifeq ($(CONFIG_TY_TKL_DISPLAY_DRM),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_display/src/tkl_display_drm.c
LOCAL_PKG_DEPS += libdrm
LOCAL_CFLAGS += -I$(TARGET_SYSROOT)/usr/include/libdrm
LOCAL_CFLAGS += -DTY_TKL_DISPLAY_DRM
else ifeq ($(CONFIG_TY_TKL_DISPLAY_FBDEV),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/tkl_display/src/tkl_display_fbdev.c
LOCAL_CFLAGS += -DTY_TKL_DISPLAY_FBDEV
else
$(error No display backend)
endif
endif

#tkl_cellular

ifeq ($(CONFIG_ENABLE_TKL_CELLULAR),y)

LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/tkl_cellular/include -I$(TKL_SRC_PATH)/src/tkl_cellular/src

LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_cellular/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")

LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/tkl_cellular/include

endif

#tkl_watchdog
ifeq ($(CONFIG_ENABLE_TKL_WATCHDOG),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_watchdog/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_CFLAGS += -Wall -DTUYA_TKL_WDT_DEV=\"$(CONFIG_TY_TKL_WATCHDOG_DEV)\"
endif

#tkl_gpio
ifeq ($(CONFIG_ENABLE_TKL_GPIO),y)
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_gpio/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")

ifeq ($(CONFIG_TY_TKL_GPIO_UPPERCASE), y)
LOCAL_CFLAGS += -DGPIO_UPPERCASE
endif
endif

#LOCAL_SHARED_LIBRARIES  += libsys_log libtkl_utils

#tkl_base
ifeq ($(CONFIG_ENABLE_TKL_BASE),y)
LOCAL_CFLAGS += \
        -I$(TKL_SRC_PATH)/tkl_base/include \
        -I$(TKL_SRC_PATH)/tkl_base/include/tuyaos \
        -I$(TKL_SRC_PATH)/tkl_base/include/platform \
        -I$(TKL_SRC_PATH)/tkl_base/src/utils \
        -I$(TKL_SRC_PATH)/tkl_base/src/input_event \
        -I$(TKL_SRC_PATH)/tkl_base/src/uart \
        -I$(TKL_SRC_PATH)/tkl_base/src  \
        -I$(TKL_SRC_PATH)/tkl_base/src/gpio \
        -I$(TKL_SRC_PATH)/tkl_base/src/pwm \
        -I$(TKL_SRC_PATH)/tkl_base/src/adc \
        -I$(TKL_SRC_PATH)/tkl_base/src/ir

ifeq ($(CONFIG_TY_TKL_BASE_GPIO_UPPERCASE), y)
LOCAL_CFLAGS += -DGPIO_UPPERCASE
endif
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_base/src/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")

endif


#nvram 
ifeq ($(CONFIG_ENABLE_NVRAM),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/nvram_kernel_driver/src/lib/nvram.c
LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/nvram_kernel_driver/include
LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/nvram_kernel_driver/include 
endif

#bsp_verify

ifeq ($(CONFIG_ENABLE_BSP_VERIFY),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/bsp_verify/src/ecdsa.c $(TKL_SRC_PATH)/bsp_verify/src/sha256.c $(TKL_SRC_PATH)/bsp_verify/src/tuya_verify.c
LOCAL_CFLAGS += -fPIC -I$(TKL_SRC_PATH)/bsp_verify/include -I$(TKL_SRC_PATH)/bsp_verify/src
endif

#swupgrade_unpack

ifeq ($(CONFIG_ENABLE_SWUPGRADE_UNPACK),y)
LOCAL_SRC_FILES += $(TKL_SRC_PATH)/swupgrade_unpack/src/handler.c  \
                   $(TKL_SRC_PATH)/swupgrade_unpack/src/handler_wh.c  \
                   $(TKL_SRC_PATH)/swupgrade_unpack/src/handler_bk.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/notifier.c     \
                   $(TKL_SRC_PATH)/swupgrade_unpack/src/property.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/swupgrade_async.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/swupgrade_network.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/swupgrade_progress.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/tuya_sha256.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/tuya_unpack.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/ipc/network_ipc.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/ipc/progress_ipc.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers/amlogic_nand_flash_handler.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers/dummy_handler.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers/nand_flash_handler.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers/nor_flash_handler.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers/rockchip_nand_flash_handler.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/3rdparty/mtd-utils/lib/libmtd.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_wh/amlogic_nand_flash_handler_wh.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_wh/dummy_handler_wh.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_wh/nand_flash_handler_wh.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_wh/nor_flash_handler_wh.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_wh/rockchip_nand_flash_handler_wh.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_bk/amlogic_nand_flash_handler_bk.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_bk/dummy_handler_bk.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_bk/nand_flash_handler_bk.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_bk/nor_flash_handler_bk.c  \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/handlers_bk/rockchip_nand_flash_handler_bk.c \
		   $(TKL_SRC_PATH)/swupgrade_unpack/src/3rdparty/mtd-utils/lib/libmtd_legacy.c

LOCAL_CFLAGS += -I$(TKL_SRC_PATH)/swupgrade_unpack/include \
	        -I$(TKL_SRC_PATH)/swupgrade_unpack/src/3rdparty/mtd-utils/include \
		-I$(TKL_SRC_PATH)/swupgrade_unpack/src

LOCAL_TUYA_SDK_INC += $(TKL_SRC_PATH)/swupgrade_unpack/include

ifeq ($(CONFIG_ENABLE_URL_INSTALL),y)
LOCAL_CFLAGS += -DENABLE_URL_INSTALL=1 -I$(TKL_SRC_PATH)/swupgrade_unpack/src \
	-I$(TKL_SRC_PATH)/swupgrade_unpack/src/3rdparty/mbedtls/crypto/include \
	-I$(TKL_SRC_PATH)/swupgrade_unpack/src/3rdparty/mbedtls/include
endif

ifeq ($(CONFIG_ENABLE_FLASH_ERASE_WRITE_SLEEP),y)
LOCAL_CFLAGS += -DENABLE_FLASH_ERASE_WRITE_SLEEP=1
endif

ifeq ($(CONFIG_ENABLE_WRITE_AHEAD),y)
LOCAL_CFLAGS += -DENABLE_WRITE_AHEAD=1
endif

ifeq ($(CONFIG_ENABLE_FLASH_ERASE_128KB_BLOCK),y)
LOCAL_CFLAGS += -DENABLE_FLASH_ERASE_128KB_BLOCK=1
endif

ifeq ($(CONFIG_TY_BSP_PACK_PARTITION_SETABLE), y)
PACK_PART_CNT=$(call qstrip,$(CONFIG_TY_BSP_PACK_PARTITION_CNT))
LOCAL_CFLAGS += -DBSP_PACK_PART_CNT=$(PACK_PART_CNT)
endif

ifeq ($(CONFIG_TUYA_BSP_VERIFY_ONBOARD_NVRAM), y)
LOCAL_CFLAGS += -DTUYA_BSP_VERIFY_ONBOARD
endif

ifeq ($(CONFIG_TUYA_BSP_SIGN_COULD),y)
LOCAL_CFLAGS += -DTUYA_BSP_SIGN_COULD
endif

endif


#tkl_rtc
ifeq ($(CONFIG_ENABLE_TKL_RTC),y)
LOCAL_CFLAGS += -I $(TKL_SRC_PATH)/tkl_rtc/include
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_rtc/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_CFLAGS += -DTY_TKL_RTC_DEV=\"$(CONFIG_TY_TKL_RTC_DEV)\"
LOCAL_SHARED_LIBRARIES  += libsys_log
endif

#tkl_media
ifeq ($(CONFIG_ENABLE_TKL_MEDIA),y)
# 模块源代码
LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_media_template/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif

LOCAL_SRC_FILES += $(shell find $(TKL_SRC_PATH)/tkl_bluetooth/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")

TUYA_SDK_INC += $(LOCAL_TUYA_SDK_INC)  # 此行勿修改
LOCAL_LDFLAGS += -ldl -lm -lpthread -lcrypt

# 生成静态库
include $(BUILD_STATIC_LIBRARY)

# 生成动态库
include $(BUILD_SHARED_LIBRARY)

# 导出编译详情
include $(OUT_COMPILE_INFO)

#---------------------------------------
