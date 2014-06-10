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
"${prefix}/ImageFormat_convert.cpp")
list(REMOVE_ITEM sources ${skipFiles})
source_group("G3D" FILES ${sources})
SET( SRCS ${SRCS} ${sources} )

FILE(GLOB vmap "${prefix}/vmap/*.*")
source_group("vMap" FILES ${vmap})
SET( SRCS ${SRCS} ${vmap} )

include_directories( ${prefix} )
add_library(${PROJECT_NAME} STATIC ${SRCS} )

#Set it to the project name.
set(G3DLITE_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "${PROJECT_NAME} Libraries")
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Dependencies")
