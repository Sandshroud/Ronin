project(dbclib)

SET( prefix ${DEPENDENCY_SOURCES}/${PROJECT_NAME} )

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp")
source_group("${PROJECT_NAME}" FILES ${sources})
SET(SRCS ${SRCS} ${sources} )

include_directories( ${GLOBAL_INCLUDE_DIRS} ${prefix} )
add_library(${PROJECT_NAME} STATIC ${SRCS} )

add_dependencies( ${PROJECT_NAME} stormlib )
target_link_libraries( ${PROJECT_NAME} ${STORMLIB_LIBRARIES} stormlib )

#Set it to the project name.
set(DBCLIB_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "${PROJECT_NAME} Libraries")
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Dependencies")
