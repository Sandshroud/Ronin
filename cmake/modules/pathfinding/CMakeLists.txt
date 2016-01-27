project(Pathfinding)

SET( prefix ${ROOT_PATH}/dependencies/SRC/pathfinding )

#Pathfinding files
FILE(GLOB pathfinding
"${prefix}/*.h"
"${prefix}/*.cpp")
source_group("Pathfinding" FILES ${pathfinding})

#Set the rest of our file sources
SET(SRCS ${SRCS} ${pathfinding} )

#Set our directories for includes
include_directories( ${GLOBAL_INCLUDE_DIRS} ${RONIN_SOURCE_DIRECTORIES} )
#Add our dependencies and our ScriptLib declaration
link_directories( ${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH} )
add_definitions(-DSCRIPTLIB)

#Add our source files
add_library( ${PROJECT_NAME} SHARED ${SRCS} )

#Set the solution folder to modules
SET_PROPERTY(TARGET Pathfinding PROPERTY FOLDER "Modules")

#Link our libraries.
add_dependencies( ${PROJECT_NAME} g3dlite recast detour mmaplib )
target_link_libraries( ${PROJECT_NAME} g3dlite recast detour mmaplib ${EXTRA_LIBS} )
