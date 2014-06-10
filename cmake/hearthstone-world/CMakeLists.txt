PROJECT(hearthstone-world)

SET( prefix ${ROOT_PATH}/src/hearthstone-world )

#Put our AI headers in a separate filter for easy access
FILE(GLOB ai_headers "${prefix}/AI/*.h")
FILE(GLOB ai_sources "${prefix}/AI/*.cpp")
source_group("AI" FILES ${ai_sources})
source_group("AI\\Headers" FILES ${ai_headers})
SET(group_sources ${group_sources} ${ai_sources} ${ai_headers} )

FILE(GLOB battleground_sources
"${prefix}/Battlegrounds/*.h"
"${prefix}/Battlegrounds/*.cpp")
source_group("Battlegrounds" FILES ${battleground_sources})
FILE(GLOB exp0_bg_sources
"${prefix}/Battlegrounds/Expansion0/*.h"
"${prefix}/Battlegrounds/Expansion0/*.cpp")
source_group("Battlegrounds\\Expansion0" FILES ${exp0_bg_sources})
FILE(GLOB exp1_bg_sources
"${prefix}/Battlegrounds/Expansion1/*.h"
"${prefix}/Battlegrounds/Expansion1/*.cpp")
source_group("Battlegrounds\\Expansion1" FILES ${exp1_bg_sources})
FILE(GLOB exp2_bg_sources
"${prefix}/Battlegrounds/Expansion2/*.h"
"${prefix}/Battlegrounds/Expansion2/*.cpp")
source_group("Battlegrounds\\Expansion2" FILES ${exp2_bg_sources})
SET(group_sources ${group_sources} ${battleground_sources} ${exp0_bg_sources} ${exp1_bg_sources} ${exp2_bg_sources})

FILE(GLOB chat_sources
"${prefix}/Chat/*.h"
"${prefix}/Chat/*.cpp")
source_group("Chat" FILES ${chat_sources})
SET(group_sources ${group_sources} ${chat_sources})

FILE(GLOB chat_command_sources
"${prefix}/Chat/Commands/*.h"
"${prefix}/Chat/Commands/*.cpp")
source_group("Chat" FILES ${chat_command_sources})
SET(group_sources ${group_sources} ${chat_command_sources})

FILE(GLOB client_coms_sources
"${prefix}/Client Communications/*.h"
"${prefix}/Client Communications/*.cpp")
source_group("Client Comms" FILES ${client_coms_sources})
SET(group_sources ${group_sources} ${client_coms_sources})

FILE(GLOB client_coms_sources
"${prefix}/Client Communications/*.h"
"${prefix}/Client Communications/*.cpp")
source_group("Client Comms" FILES ${client_coms_sources})
SET(group_sources ${group_sources} ${client_coms_sources})

FILE(GLOB client_coms_sources
"${prefix}/Client Communications/Packet Handlers/*.h"
"${prefix}/Client Communications/Packet Handlers/*.cpp")
source_group("Client Comms\\Packet Handlers" FILES ${client_coms_sources})
SET(group_sources ${group_sources} ${client_coms_sources})

FILE(GLOB intersystem_sources
"${prefix}/Intersystem/*.h"
"${prefix}/Intersystem/*.cpp")
source_group("Interserver" FILES ${intersystem_sources})
SET(group_sources ${group_sources} ${intersystem_sources})

FILE(GLOB intersystem_logon_sources
"${prefix}/Intersystem/Logon/*.h"
"${prefix}/Intersystem/Logon/*.cpp")
source_group("Interserver\\Logon" FILES ${intersystem_logon_sources})
SET(group_sources ${group_sources} ${intersystem_logon_sources})

FILE(GLOB intersystem_management_sources
"${prefix}/Intersystem/Management/*.h"
"${prefix}/Intersystem/Management/*.cpp")
source_group("Interserver\\Management" FILES ${intersystem_management_sources})
SET(group_sources ${group_sources} ${intersystem_management_sources})

FILE(GLOB intersystem_management_guild_sources
"${prefix}/Intersystem/Management/Guild/*.h"
"${prefix}/Intersystem/Management/Guild/*.cpp")
source_group("Interserver\\Management\\Guild" FILES ${intersystem_management_guild_sources})
SET(group_sources ${group_sources} ${intersystem_management_guild_sources})

FILE(GLOB intersystem_management_item_sources
"${prefix}/Intersystem/Management/Items/*.h"
"${prefix}/Intersystem/Management/Items/*.cpp")
source_group("Interserver\\Management\\Items" FILES ${intersystem_management_item_sources})
SET(group_sources ${group_sources} ${intersystem_management_item_sources})

FILE(GLOB intersystem_voice_sources
"${prefix}/Intersystem/Voice/*.h"
"${prefix}/Intersystem/Voice/*.cpp")
source_group("Interserver\\Voice Chat" FILES ${intersystem_voice_sources})
SET(group_sources ${group_sources} ${intersystem_voice_sources})

FILE(GLOB mainframe_sources
"${prefix}/Mainframe/*.h"
"${prefix}/Mainframe/*.cpp")
source_group("Mainframe" FILES ${mainframe_sources})
SET(group_sources ${group_sources} ${mainframe_sources})

FILE(GLOB mainframe_resouce_sources
"${prefix}/Mainframe/Resources/*.*")
source_group("Mainframe\\Resources" FILES ${mainframe_resouce_sources})
SET(group_sources ${group_sources} ${mainframe_resouce_sources})

FILE(GLOB map_system_sources
"${prefix}/Map System/*.h"
"${prefix}/Map System/*.cpp")
source_group("Map System" FILES ${map_system_sources})
SET(group_sources ${group_sources} ${map_system_sources})

FILE(GLOB map_system_environment_sources
"${prefix}/Map System/Environment/*.h"
"${prefix}/Map System/Environment/*.cpp")
source_group("Map System\\Environment" FILES ${map_system_environment_sources})
SET(group_sources ${group_sources} ${map_system_environment_sources})

FILE(GLOB map_system_map_interface_sources
"${prefix}/Map System/Map Interface/*.h"
"${prefix}/Map System/Map Interface/*.cpp")
source_group("Map System\\Map Interface" FILES ${map_system_map_interface_sources})
SET(group_sources ${group_sources} ${map_system_map_interface_sources})

FILE(GLOB map_system_map_managers_sources
"${prefix}/Map System/Map Managers/*.h"
"${prefix}/Map System/Map Managers/*.cpp")
source_group("Map System\\Map Managers" FILES ${map_system_map_managers_sources})
SET(group_sources ${group_sources} ${map_system_map_managers_sources})

FILE(GLOB map_system_map_tiles_sources
"${prefix}/Map System/Map Tiles/*.h"
"${prefix}/Map System/Map Tiles/*.cpp")
source_group("Map System\\Map Tiles" FILES ${map_system_map_tiles_sources})
SET(group_sources ${group_sources} ${map_system_map_tiles_sources})

FILE(GLOB object_sources
"${prefix}/Objects/*.h"
"${prefix}/Objects/*.cpp")
source_group("Objects" FILES ${object_sources})
SET(group_sources ${group_sources} ${object_sources})

FILE(GLOB object_unit_sources
"${prefix}/Objects/Units/*.h"
"${prefix}/Objects/Units/*.cpp")
source_group("Objects\\Units" FILES ${object_unit_sources})
SET(group_sources ${group_sources} ${object_unit_sources})

FILE(GLOB player_system_sources
"${prefix}/Player Systems/*.h"
"${prefix}/Player Systems/*.cpp")
source_group("Player Systems" FILES ${player_system_sources})
SET(group_sources ${group_sources} ${player_system_sources})

FILE(GLOB script_sources
"${prefix}/ScriptInterface/*.h"
"${prefix}/ScriptInterface/*.cpp")
source_group("ScriptInterface" FILES ${script_sources})
SET(group_sources ${group_sources} ${script_sources})

FILE(GLOB spell_sources
"${prefix}/Spell/*.h"
"${prefix}/Spell/*.cpp")
source_group("Spell" FILES ${spell_sources})
SET(group_sources ${group_sources} ${spell_sources})

FILE(GLOB spell_fixes_sources
"${prefix}/Spell/Spellfixes/*.h"
"${prefix}/Spell/Spellfixes/*.cpp")
source_group("Spell\\Spell Fixes" FILES ${spell_fixes_sources})
SET(group_sources ${group_sources} ${spell_fixes_sources})

FILE(GLOB sources "${prefix}/*.h" "${prefix}/*.cpp"
"${ROOT_PATH}/src/hearthstone-shared/MemAllocator.*")
source_group("System" FILES ${sources})

#Remove already added files from our main source files
list(REMOVE_ITEM sources ${group_sources})
#Set the rest of our file sources
SET(SRCS ${SRCS} ${sources} ${group_sources} )
#Set our precompiled header data
include(PCHSupport)
#Set our directories for includes
include_directories( ${GLOBAL_INCLUDE_DIRS} ${HEARTHSTONE_SOURCE_DIRECTORIES} )

link_directories( ${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH} )
#Add our sources to our project files
add_executable( ${PROJECT_NAME} ${SRCS} )
#Redefine our precompiled header inside our sources
IF( CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} StdAfx.h ${prefix}/StdAfx.cpp)
ENDIF()

#Link our hearthstone-shared lib.
add_dependencies( ${PROJECT_NAME} hearthstone-shared g3dlite recast detour pcre zlib )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES} hearthstone-shared g3dlite recast detour pcre zlib ${EXTRA_LIBS} )

#Set our resources
set_source_files_properties(Icon.ico resources.rc PROPERTIES LANGUAGE RC)
#Set our compile information
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)
