
add_subdirectory(dataextractor)
add_subdirectory(itemdbcgenerator)
add_subdirectory(mmapbuilder)
add_subdirectory(vmapassembler)
add_subdirectory(vmapextractor)

#Copy libmysql for tool usage
FILE(COPY "${DEPENDENCY_DLLS}/libmysql.dll" DESTINATION "${OUTPUT_DIRECTORY}/Tools")

#Copy .bat file for vmap generation
if(NOT EXISTS "${OUTPUT_DIRECTORY}/Tools/vmap_simple.bat" AND EXISTS "${ROOT_PATH}/src/tools/vmap_simple.bat")
FILE(COPY "${ROOT_PATH}/src/tools/vmap_simple.bat" DESTINATION "${OUTPUT_DIRECTORY}/Tools")
endif()
