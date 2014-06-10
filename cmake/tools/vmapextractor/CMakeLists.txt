PROJECT(VMapExtractor)

SET( prefix "${ROOT_PATH}/src/tools/vmap extractor")

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp"
"${prefix}/loadlib/*.h"
"${prefix}/loadlib/*.cpp")
source_group("${PROJECT_NAME}" FILES ${sources})

SET( SRCS ${SRCS} ${sources} )
SET( SRCS ${SRCS} "${ROOT_PATH}/src/tools/Icon.ico" "${ROOT_PATH}/src/tools/resources.rc")

include_directories( ${GLOBAL_INCLUDE_DIRS} )
link_directories( ${EXTRA_LIBS_PATH} ${DEPENDENCY_LIBS} )
add_executable( ${PROJECT_NAME} ${SRCS} )
add_dependencies( ${PROJECT_NAME} pcre zlib stormlib )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES} pcre zlib stormlib )
#Set the solution folder to tools
SET_PROPERTY(TARGET VMapExtractor PROPERTY FOLDER "Tools")
#Set the output folder to bin/tools
set_target_properties( VMapExtractor PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/Tools")
foreach(buildtype IN ITEMS ${CMAKE_CONFIGURATION_TYPES} )
  string(TOUPPER "${buildtype}" BUILD_DATA)
  set_target_properties( VMapExtractor PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${BUILD_DATA}" "${OUTPUT_DIRECTORY}/Tools")
endforeach()
