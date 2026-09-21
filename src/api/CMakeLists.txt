file(GLOB_RECURSE SOURCES ./*.c  ./*.cpp)


include_directories(../include/anyka)
include_directories(../include/api)
include_directories(../include/freetype)
include_directories(../include/libmad)
include_directories(../include/tuya)
include_directories(../lvgl)

include_directories(../include/live555/BasicUsageEnvironment/include)
include_directories(../include/live555/groupsock/include)
include_directories(../include/live555/liveMedia/include)
include_directories(../include/live555/UsageEnvironment/include)

include_directories(../tuya_batch_production_sdk/include/libxls)
include_directories(../tuya_batch_production_sdk/include/iconv)
include_directories(../tuya_batch_production_sdk/xls)

include_directories(./queue)
include_directories(./video)    
include_directories(./common)    
include_directories(./audio)
include_directories(./network)
include_directories(./png_lib)
include_directories(./freetype)
include_directories(./wlan)

add_library(leo_api_lib ${SOURCES} )