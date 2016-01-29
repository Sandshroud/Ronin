PROJECT(g3dlite)

SET(prefix ${DEPENDENCY_SOURCES}/${PROJECT_NAME} )

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp"
"${prefix}/*.inl")
SET(skipFiles
"${prefix}/GImage.h"
"${prefix}/GImage.cpp"
"${prefix}/GImage_bayer.cpp"
"${prefix}/GImage_bmp.cpp"
"${prefix}/GImage_jpeg.cpp"
"${prefix}/GImage_png.cpp"
"${prefix}/GImage_ppm.cpp"
"${prefix}/GImage_tga.cpp"
"${prefix}/ImageFormat.h"
"${prefix}/ImageFormat.cpp"
"${prefix}/ImageFormat_convert.cpp"
"${prefix}/Stopwatch.h"
"${prefix}/Stopwatch.cpp")
list(REMOVE_ITEM sources ${skipFiles})
source_group("G3D" FILES ${sources})
SET( SRCS ${SRCS} ${sources} )

include_directories( ${GLOBAL_INCLUDE_DIRS} ${prefix} )
add_library(${PROJECT_NAME} STATIC ${SRCS} )

SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Dependencies")
