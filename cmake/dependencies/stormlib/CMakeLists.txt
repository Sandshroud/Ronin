project(stormlib)

SET( prefix ${DEPENDENCY_SOURCES}/${PROJECT_NAME} )

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp"
"${prefix}/adpcm/*.h"
"${prefix}/adpcm/*.cpp"
"${prefix}/huffman/*.h"
"${prefix}/huffman/*.cpp"
"${prefix}/jenkins/*.h"
"${prefix}/jenkins/*.c"
"${prefix}/libtomcrypt/src/hashes/*.c"
"${prefix}/libtomcrypt/src/headers/*.h"
"${prefix}/libtomcrypt/src/math/*.c"
"${prefix}/libtomcrypt/src/misc/*.c"
"${prefix}/libtomcrypt/src/pk/asn1/*.c"
"${prefix}/libtomcrypt/src/pk/ecc/*.c"
"${prefix}/libtomcrypt/src/pk/pkcs1/*.c"
"${prefix}/libtomcrypt/src/pk/rsa/*.c"
"${prefix}/libtommath/*.h"
"${prefix}/libtommath/*.c"
"${prefix}/lzma/C/*.h"
"${prefix}/lzma/C/*.c"
"${prefix}/pklib/*.h"
"${prefix}/pklib/*.c"
"${prefix}/sparse/*.h"
"${prefix}/sparse/*.cpp")
source_group("${PROJECT_NAME}" FILES ${sources})
SET(SRCS ${SRCS} ${sources} )

include_directories( ${GLOBAL_INCLUDE_DIRS} ${prefix} )
add_library(${PROJECT_NAME} STATIC ${SRCS} )

add_dependencies( ${PROJECT_NAME} bzip2 zlib )
target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES} bzip2 zlib )

#Set it to the project name.
set(STORMLIB_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "${PROJECT_NAME} Libraries")
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Dependencies")
