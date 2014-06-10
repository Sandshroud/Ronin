PROJECT(hearthstone-shared)

SET( prefix ${ROOT_PATH}/src/hearthstone-shared )

FILE(GLOB auth_files
"${prefix}/Auth/*.h"
"${prefix}/Auth/*.cpp")
source_group("Auth" FILES ${auth_files})
SET(SRCS ${SRCS} ${auth_files})

FILE(GLOB client_files
"${prefix}/Client/*.h"
"${prefix}/Client/*.cpp")
source_group("Client" FILES ${client_files})
SET(SRCS ${SRCS} ${client_files} )

FILE(GLOB config_files
"${prefix}/Config/*.h"
"${prefix}/Config/*.cpp")
source_group("Config" FILES ${config_files})
SET(SRCS ${SRCS} ${config_files} )

IF(TRUE)
FILE(GLOB unixconfig
"${prefix}/Config/unix/*.c"
"${prefix}/Config/unix/*.h")
source_group("Config" FILES ${unixconfig})

SET(SRCS ${SRCS} ${unixconfig} )
ENDIF()

FILE(GLOB console_files
"${prefix}/Console/*.h"
"${prefix}/Console/*.cpp")
source_group("Console" FILES ${console_files})
SET(SRCS ${SRCS} ${console_files} )

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp")
source_group("Util" FILES ${sources})
SET(SRCS ${SRCS} ${sources})

include_directories( ${GLOBAL_INCLUDE_DIRS} )
link_directories( ${DEPENDENCY_DLLS} ${DEPENDENCY_LIBS} )

ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SRCS} )

add_dependencies( ${PROJECT_NAME} g3dlite recast detour pcre zlib consolelog database dbclib network threading )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES} g3dlite recast detour pcre zlib consolelog database dbclib network threading ${EXTRA_LIBS} )

#Set our compile information
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS SHARED_LIB_BUILDER)
