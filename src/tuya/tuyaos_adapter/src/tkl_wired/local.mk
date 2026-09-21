# 当前文件所在目录
LOCAL_PATH := $(call my-dir)

#---------------------------------------

# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
LOCAL_MODULE := $(notdir $(LOCAL_PATH))

# 申明模块产物为SDK
LOCAL_MODULE_TAGS := sdk

# 模块对外头文件（只能是目录）
# 加载至CFLAGS中提供给其他组件使用；打包进SDK产物中；
# LOCAL_TUYA_SDK_INC := $(LOCAL_PATH)/include

# 模块对外CFLAGS：其他组件编译时可感知到
LOCAL_TUYA_SDK_CFLAGS :=

NETWORK_SRC_PATH = "common"
ifeq ($(CONFIG_TUYA_PRODUCT_RELEASE_NAME), "x86_64")
NETWORK_SRC_PATH= "x86_64"
else ifeq ($(CONFIG_TUYA_PRODUCT_RELEASE_NAME), "rtl8197")
NETWORK_SRC_PATH="rtl8197"

else ifeq ($(CONFIG_TUYA_PRODUCT_RELEASE_NAME), "tpp01z")
NETWORK_SRC_PATH="rk3308"

else ifeq ($(CONFIG_TUYA_PRODUCT_RELEASE_NAME), "ssd20x")
NETWORK_SRC_PATH="common"
MY_LOCAL_CFLAGS := -DSSD20X

else ifeq ($(CONFIG_TUYA_PRODUCT_RELEASE_NAME), "ssd212")
NETWORK_SRC_PATH="common"
MY_LOCAL_CFLAGS := -DSSD20X
endif


$(info "PRODUCT_RELEASE_NAME:"$(CONFIG_TUYA_PRODUCT_RELEASE_NAME))
$(info "NETWORK_SRC_PATH:"$(NETWORK_SRC_PATH))

# 模块源代码
LOCAL_SRC_FILES := $(shell find $(LOCAL_PATH)/src/$(NETWORK_SRC_PATH) -name "*.c" -o -name "*.cpp" -o -name "*.cc")

# 模块的 CFLAGS
LOCAL_CFLAGS := -Wall -fPIC $(MY_LOCAL_CFLAGS)
LOCAL_CFLAGS += \
		-I$(LOCAL_PATH)/include

LOCAL_LDFLAGS := -lpthread

# 全局变量赋值
TUYA_SDK_INC += $(LOCAL_TUYA_SDK_INC)  # 此行勿修改
TUYA_SDK_CFLAGS += $(LOCAL_TUYA_SDK_CFLAGS)  # 此行勿修改

# 生成静态库
include $(BUILD_STATIC_LIBRARY)

# 生成动态库
include $(BUILD_SHARED_LIBRARY)

ifeq ($(CONFIG_TUYA_TKL_WIRED_TEST),y)
include $(LOCAL_PATH)/test/local_sub.mk
endif

#---------------------------------------

