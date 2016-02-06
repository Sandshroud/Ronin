PROJECT(DataExtractor)

SET( prefix "${ROOT_PATH}/src/tools/data extractor")

FILE(GLOB sources
"${prefix}/*.h"
"${prefix}/*.cpp")
source_group("${PROJECT_NAME}" FILES ${sources})

SET( SRCS ${SRCS} ${sources} )
SET( SRCS ${SRCS} "${ROOT_PATH}/src/tools/Icon.ico" "${ROOT_PATH}/src/tools/resources.rc")

include_directories( ${GLOBAL_INCLUDE_DIRS} )
link_directories( ${EXTRA_LIBS_PATH} ${DEPENDENCY_LIBS} )
add_executable( ${PROJECT_NAME} ${SRCS} )
add_dependencies( ${PROJECT_NAME} bzip2 pcre zlib threading stormlib g3dlite vmaplib )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} bzip2 pcre zlib threading stormlib g3dlite vmaplib )

foreach(buildtype IN ITEMS ${CMAKE_CONFIGURATION_TYPES} )
  string(TOUPPER "${buildtype}" BUILD_DATA)
  set_target_properties( DataExtractor PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${BUILD_DATA}" "${OUTPUT_DIRECTORY}/Tools")
endforeach()
