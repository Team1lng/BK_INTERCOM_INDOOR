add_compile_options(-w)

add_definitions(
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
	${CMAKE_CURRENT_SOURCE_DIR}/include/base/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/tal_system/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/tal_system_service/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_api_ipc/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_wifi/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_devos/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/lib_cJSON/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_netmgr/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/base_media/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_ipc_core/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_streaming_p2p/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_video_msg/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_cloud_storage/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/tal_security/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_local_storage/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_ring_buffer/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_rule/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_ipc_tmm_control/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_api_iot/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_tmm_link/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_ptz/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/tal_media/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/lib_tls/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_tuya_cloud/include/tls
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/utilities/include/
	${CMAKE_CURRENT_SOURCE_DIR}/app/app_main/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/security/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/bluetooth/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/init/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/system/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/wifi/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/media/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/storage/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/pm/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/wired/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/rtc/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/watchdog/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/flash/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/display/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/adc/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/dac/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/i2c/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/i2s/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/gpio/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/pwm/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/spi/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/register/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/timer/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/include/uart/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_ipc_ai/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_ai_basic/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_bt/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/base_event/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/tal_system_service/include/utilities/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/tal_bluetooth/include/
	${CMAKE_CURRENT_SOURCE_DIR}/include/components/svc_aov_buffer/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/bsp_cryptfs/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/nvram_kernel_driver/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/tkl_ir/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/tkl_input_event/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/sys_log/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/tkl_utils/include/
	${CMAKE_CURRENT_SOURCE_DIR}/tuyaos_adapter/src/tkl_wifi/include/

)

# FILE(GLOB TUYA_SRC
# 	app/app_main/src/ty_sdk_main.c
# 	tuyaos_adapter/include/utilities/src/tuya_list.c
# 	tuyaos_adapter/src/tkl_bluetooth.c
# 	tuyaos_adapter/include/init/src/tkl_init_system.c
# 	tuyaos_adapter/src/tkl_fs.c
# 	tuyaos_adapter/src/tkl_queue.c
# 	tuyaos_adapter/src/tkl_semaphore.c
# 	tuyaos_adapter/src/tkl_memory.c
# 	tuyaos_adapter/src/tkl_mutex.c
# 	tuyaos_adapter/src/tkl_thread.c
# 	tuyaos_adapter/src/tkl_ota.c
# 	tuyaos_adapter/src/tkl_output.c
# 	tuyaos_adapter/src/tkl_system.c
# 	tuyaos_adapter/src/tkl_sleep.c
# 	app/app_main/src/ty_sdk_sd_card.c
# )

file(GLOB_RECURSE TUYA_SRC ./*.c)

add_library(tuya STATIC ${TUYA_SRC})