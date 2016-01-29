project(dbclib)

SET( prefix ${DEPENDENCY_SOURCES}/${PROJECT_NAME} )

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp")
source_group("${PROJECT_NAME}" FILES ${sources})
SET(SRCS ${SRCS} ${sources} )

include_directories( ${GLOBAL_INCLUDE_DIRS} ${prefix} )
add_library(${PROJECT_NAME} STATIC ${SRCS} )

SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Dependencies")
