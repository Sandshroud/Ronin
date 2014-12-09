project(vmaplib)

SET( prefix ${DEPENDENCY_SOURCES}/${PROJECT_NAME} )

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp")
source_group("${PROJECT_NAME}" FILES ${sources})
SET(SRCS ${SRCS} ${sources} )

include_directories( ${GLOBAL_INCLUDE_DIRS} ${prefix} )
add_library(${PROJECT_NAME} STATIC ${SRCS} )

add_dependencies( ${PROJECT_NAME} g3dlite )
target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES} g3dlite )

#Set it to the project name.
set(THREADING_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "${PROJECT_NAME} Libraries")
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Dependencies")
