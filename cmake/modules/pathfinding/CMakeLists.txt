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
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Modules")
#Set the output folder to modules subdirectory
set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/Modules")
foreach(buildtype IN ITEMS ${CMAKE_CONFIGURATION_TYPES} )
  string(TOUPPER "${buildtype}" BUILD_DATA)
  set_target_properties( ${PROJECT_NAME} PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${BUILD_DATA}" "${OUTPUT_DIRECTORY}/Modules")
endforeach()

#Link our libraries.
add_dependencies( ${PROJECT_NAME} g3dlite recast detour mmaplib )
target_link_libraries( ${PROJECT_NAME} g3dlite recast detour mmaplib ${EXTRA_LIBS} )
