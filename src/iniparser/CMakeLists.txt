file(GLOB_RECURSE SOURCES ./src/*.c)
#add_library(lv_drivers STATIC ${SOURCES})
add_library(iniparser ${SOURCES})