#SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_C_FLAGS_RELEASE  "$ENV{CXXFLAGS} -Os -Wall -std=gnu11 -s"  ) # -Werror
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Os -Wall -std=c++1y -s ") # -Werror

find_package(Threads)

aux_source_directory(. DIR_SRCS)
aux_source_directory(./layout DIR_SRCS)

file(REMOVE ./layout/resource/rom.bin)
file(REMOVE ./layout/resource/rom.h)
file(REMOVE ../language.xls)
# file(REMOVE ../upgrade/app/rings/*)#file(REMOVE ...)命令不支持通配符（wildcard）匹配。因此，直接使用file(REMOVE ../upgrade/app/rings/*)是不能达到预期效果的
# 这段代码将使用file命令和GLOB宏获取指定路径下的所有文件，并使用foreach循环遍历每个文件，然后使用file命令的REMOVE选项将其删除。
file(GLOB RINGS_FILES "../upgrade/app/rings/*")
foreach(RINGS_FILE ${RINGS_FILES})
    file(REMOVE ${RINGS_FILE})
endforeach()

# file(GLOB RINGS_FILES "../upgrade/app/rings/*")
# file(REMOVE ${RINGS_FILES})

aux_source_directory(./layout/resource/res_boke DIR_SRCS)
file(COPY ./layout/resource/res_boke/rom.h DESTINATION ./layout/resource/)
file(COPY ./layout/resource/res_boke/rom.bin DESTINATION ./layout/resource/)
file(COPY ../upgrade/ring/normal/rings/ DESTINATION ../upgrade/app/rings/)
file(COPY ../upgrade/language/bcom/intercom/language.xls DESTINATION ../)

# aux_source_directory(./layout/resource/icon_font/output DIR_SRCS) 

aux_source_directory(./api/live555 DIR_SRCS) 
#aux_source_directory(./tuya_batch_production_sdk DIR_SRCS) 
#message("DIR_SRC : ${DIR_SRCS}")


add_subdirectory(lvgl)        #添加目录 lvgl
add_subdirectory(lv_drivers)  #添加目录 lv_drivers
add_subdirectory(lv_qrcode)  #添加目录 lv_qrcode
add_subdirectory(api)  #添加目录 api
add_subdirectory(tuya_batch_production_sdk)  #添加目录 tuya_batch_production_sdk
add_subdirectory(iniparser)  
# add_subdirectory(sqlite)  

include_directories(./include/anyka)  
include_directories(./include/api)  
include_directories(./include/api/video) 
include_directories(./include/freetype)  
include_directories(./include/libmad)  
include_directories(./include/tuya)
include_directories(./include/g711)

include_directories(./include/live555/BasicUsageEnvironment/include)  
include_directories(./include/live555/groupsock/include)  
include_directories(./include/live555/liveMedia/include)  
include_directories(./include/live555/UsageEnvironment/include)  



include_directories(./lvgl/) 
include_directories(./lv_drivers/)  
include_directories(./lv_qrcode/)  
include_directories(./layout/) 

include_directories(./api/freetype/) 
include_directories(./api/network/) 
include_directories(./api/tcp_network/) 
include_directories(./iniparser/src)  
include_directories(./tuya_batch_production_sdk/include/libxls) 
include_directories(./tuya_batch_production_sdk/include/iconv) 
include_directories(./tuya_batch_production_sdk/xls) 
include_directories(./onvif/include) 

# include_directories(./sqlite/build/include) 

#set(VAR lv_examples_lib)
set(VAR ${VAR} lv_qrcode_lib)
set(VAR ${VAR} lvgl_lib)
set(VAR ${VAR} lv_drivers_lib)
set(VAR ${VAR} leo_api_lib)
set(VAR ${VAR} iniparser)
set(VAR ${VAR} xls)
set(ANYKA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/anyka)

add_executable(${leo_proj} ${DIR_SRCS})  #链接生成文件${leo_proj} 由DIR_SRC链接。

message("VAR : ${VAR}")
target_link_libraries(${leo_proj} ${VAR})   #设置链接${leo_proj}的 依赖关系 ，依赖 VAR = lvgl_lib lv_drivers

target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_tde.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_common.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_thread.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_mem.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_osal.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_log.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_dbg.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libmpi_vdec.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libakv_decode.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libmpi_venc.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libakv_encode.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_ao.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_vqe.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libakaudiofilter.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_ai.so)


target_link_libraries(${leo_proj} -Wl,--start-group

	# stand lib
	m
	pthread
	dl

	# system lib
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/lib/libbase64.a
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/lib/libmxml.a
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/lib/libonvif.a
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/lib/libsha1.a
	-Wl,--end-group)

# 调音所需库，不调音可以屏蔽
if(${CMAKE_ATS_OPEN})
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libakaudiocodec.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libapp_ats.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libmpi_aenc.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libmpi_adec.so)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/libplat_timer.so)
endif()

target_link_libraries(${leo_proj} ${ANYKA_PATH}/../freetype/libfreetype.a)
#涂鸦升级库
target_link_libraries(${leo_proj} ${ANYKA_PATH}/../mad/libmad.a)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/../jpg/libjpeg.a)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/../g711/libg711.a)
target_link_libraries(${leo_proj} ${ANYKA_PATH}/../resample/libsamplerate.a)
#target_link_libraries(${leo_proj} ${CMAKE_CURRENT_SOURCE_DIR}/tuya_batch_production_sdk/libtuya_batch.a)#吴乾仕 MODIFY
#target_link_libraries(${leo_proj} ${CMAKE_CURRENT_SOURCE_DIR}/tuya_batch_production_sdk/lib/libxls.a)#吴乾仕 MODIFY
target_link_libraries(${leo_proj} ${CMAKE_CURRENT_SOURCE_DIR}/iniparser/libiniparser.a)#吴乾仕 MODIFY

set(LIVE555_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/live555)
target_link_libraries(${leo_proj}  -Wl,--start-group 
${LIVE555_PATH}/libBasicUsageEnvironment.a
${LIVE555_PATH}/libgroupsock.a
${LIVE555_PATH}/libliveMedia.a
${LIVE555_PATH}/libUsageEnvironment.a
-Wl,--end-group)

target_link_libraries(${leo_proj}  -Wl,--start-group 
#${ANYKA_PATH}/../tuya/libtuya_ipc.a)
#target_link_libraries(${leo_proj} ${ANYKA_PATH}/../tuya/libmbedtls.a)
#target_link_libraries(${leo_proj} ${ANYKA_PATH}/../../api/libleo_api_lib.a)
${ANYKA_PATH}/../tuya/libtuya_ipc.a
${ANYKA_PATH}/../tuya/libmbedtls.a
${ANYKA_PATH}/../../api/libleo_api_lib.a
${ANYKA_PATH}/../iconv/libiconv.a
${CMAKE_CURRENT_SOURCE_DIR}/tuya_batch_production_sdk/xls/libxls.a
-Wl,--end-group)
