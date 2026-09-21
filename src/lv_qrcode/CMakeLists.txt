file(GLOB_RECURSE SOURCES ./*.c)
#add_library(lv_qrcode STATIC ${SOURCES})
include_directories(../lv_drivers/) 
add_library(lv_qrcode_lib ${SOURCES})
