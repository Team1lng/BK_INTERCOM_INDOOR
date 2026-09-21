# 当前文件所在目录
LOCAL_PATH := $(call my-dir)

#---------------------------------------

# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
LOCAL_MODULE := $(notdir $(LOCAL_PATH))

# 输出到bsp_sdk目录
LOCAL_MODULE_TAGS := sdk

# 模块对外头文件（只能是目录）
# 加载至CFLAGS中提供给其他组件使用；打包进SDK产物中；
# LOCAL_TUYA_SDK_INC := $(LOCAL_PATH)/include

# 模块对外CFLAGS：其他组件编译时可感知到
LOCAL_TUYA_SDK_CFLAGS :=

# 模块源代码
# 本地src目录仅用于通用的linux系统默认实现，其他的实现在vendor的对应原厂开发包里
ifeq ($(CONFIG_OPERATING_SYSTEM), 100)
LOCAL_SRC_FILES := $(shell find $(LOCAL_PATH)/src -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif

# 模块内部CFLAGS：仅供本组件使用
LOCAL_CFLAGS := -fPIC -Wall -I$(LOCAL_PATH)/include

ifeq ($(CONFIG_ENABLE_GET_FREE_HEAPP),y)
LOCAL_CFLAGS += -DENABLE_GET_FREE_HEAP
endif

ifeq ($(CONFIG_ENABLE_PRIORITY_SCHED),y)
LOCAL_CFLAGS += -DENABLE_PRIORITY_SCHED
endif

ifeq ($(CONFIG_ENABLE_THREAD_STACK_SIZE),y)
LOCAL_CFLAGS += -DENABLE_THREAD_STACK_SIZE
endif

ifeq ($(CONFIG_ENABLE_LOG_OUTPUT_FORMAT),y)
LOCAL_CFLAGS += -DENABLE_LOG_OUTPUT_FORMAT
endif


LOCAL_CFLAGS += -DCREATE_RECURSION_MUTEX

# 全局变量赋值
TUYA_SDK_INC += $(LOCAL_TUYA_SDK_INC)  # 此行勿修改
TUYA_SDK_CFLAGS += $(LOCAL_TUYA_SDK_CFLAGS)  # 此行勿修改

# 生成静态库
include $(BUILD_STATIC_LIBRARY)

# 生成动态库
include $(BUILD_SHARED_LIBRARY)

# 导出编译详情
include $(OUT_COMPILE_INFO)
