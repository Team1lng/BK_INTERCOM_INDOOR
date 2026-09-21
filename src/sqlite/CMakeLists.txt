project(sqlite3)

find_package(Threads)

include_directories(./)

add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
add_library(sqlite::data ALIAS ${PROJECT_NAME})

add_executable(sqlite sqlite3.c shell.c sqlite3.h sqlite3ext.h)

add_definitions(-DSQLITE_ENABLE_RTREE)
#add_definitions(-DSQLITE_ENABLE_FTS4)
#add_definitions(-DSQLITE_ENABLE_FTS5)
#add_definitions(-DSQLITE_ENABLE_JSON1)
#add_definitions(-DSQLITE_ENABLE_RBU)
#add_definitions(-DSQLITE_ENABLE_STAT4)


target_link_libraries(sqlite m ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
install(TARGETS sqlite RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

install(FILES sqlite3.h sqlite3ext.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build/include)
install(TARGETS sqlite3 LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build/lib ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)