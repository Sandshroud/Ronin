PROJECT(ronin-world)

SET( prefix ${ROOT_PATH}/src/ronin-world )

#Put our AI headers in a separate filter for easy access
FILE(GLOB ai_sources
"${prefix}/AI/*.h"
"${prefix}/AI/*.cpp")
source_group("AI" FILES ${ai_sources})
SET(group_sources ${group_sources} ${ai_sources} )

FILE(GLOB battleground_sources
"${prefix}/Battlegrounds/*.h"
"${prefix}/Battlegrounds/*.cpp")
source_group("Battlegrounds" FILES ${battleground_sources})
SET(group_sources ${group_sources} ${battleground_sources})

FILE(GLOB chat_sources
"${prefix}/Chat/*.h"
"${prefix}/Chat/*.cpp")
source_group("Chat" FILES ${chat_sources})
SET(group_sources ${group_sources} ${chat_sources})

FILE(GLOB gm_system_sources
"${prefix}/GM Systems/*.h"
"${prefix}/GM Systems/*.cpp")
source_group("GM Systems" FILES ${gm_system_sources})
SET(group_sources ${group_sources} ${gm_system_sources})

FILE(GLOB gm_system_command_sources
"${prefix}/GM Systems/GMCommands/*.h"
"${prefix}/GM Systems/GMCommands/*.cpp")
source_group("GM Systems\\Commands" FILES ${gm_system_command_sources})
SET(group_sources ${group_sources} ${gm_system_command_sources})

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

FILE(GLOB item_sources
"${prefix}/Items/*.h"
"${prefix}/Items/*.cpp")
source_group("Items" FILES ${item_sources})
SET(group_sources ${group_sources} ${item_sources})

FILE(GLOB mainframe_sources
"${prefix}/Mainframe/*.h"
"${prefix}/Mainframe/*.cpp")
source_group("Mainframe" FILES ${mainframe_sources})
SET(group_sources ${group_sources} ${mainframe_sources})

FILE(GLOB mainframe_interserver_sources
"${prefix}/Mainframe/InterServer/*.h"
"${prefix}/Mainframe/InterServer/*.cpp")
source_group("Mainframe\\InterServer" FILES ${mainframe_interserver_sources})
SET(group_sources ${group_sources} ${mainframe_interserver_sources})

FILE(GLOB mainframe_interserver_logon_sources
"${prefix}/Mainframe/InterServer/Logon/*.h"
"${prefix}/Mainframe/InterServer/Logon/*.cpp")
source_group("Mainframe\\Interserver\\Logon" FILES ${mainframe_interserver_logon_sources})
SET(group_sources ${group_sources} ${mainframe_interserver_logon_sources})

FILE(GLOB mainframe_interserver_voice_sources
"${prefix}/Mainframe/InterServer/Voice/*.h"
"${prefix}/Mainframe/InterServer/Voice/*.cpp")
source_group("Mainframe\\Interserver\\Voice Chat" FILES ${mainframe_interserver_voice_sources})
SET(group_sources ${group_sources} ${mainframe_interserver_voice_sources})

FILE(GLOB mainframe_management_sources
"${prefix}/Mainframe/Management/*.h"
"${prefix}/Mainframe/Management/*.cpp")
source_group("Mainframe\\Management" FILES ${mainframe_management_sources})
SET(group_sources ${group_sources} ${mainframe_management_sources})

FILE(GLOB mainframe_management_events_sources
"${prefix}/Mainframe/Management/Events/*.h"
"${prefix}/Mainframe/Management/Events/*.cpp")
source_group("Mainframe\\Management\\Events" FILES ${mainframe_management_events_sources})
SET(group_sources ${group_sources} ${mainframe_management_events_sources})

FILE(GLOB mainframe_management_guild_sources
"${prefix}/Mainframe/Management/Guild/*.h"
"${prefix}/Mainframe/Management/Guild/*.cpp")
source_group("Mainframe\\Management\\Guild" FILES ${mainframe_management_guild_sources})
SET(group_sources ${group_sources} ${mainframe_management_guild_sources})

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

FILE(GLOB map_system_map_managers_instance_scripts_sources
"${prefix}/Map System/Map Managers/InstanceScripts/*.h"
"${prefix}/Map System/Map Managers/InstanceScripts/*.cpp")
source_group("Map System\\Map Managers\\Instance Scripts" FILES ${map_system_map_managers_instance_scripts_sources})
SET(group_sources ${group_sources} ${map_system_map_managers_instance_scripts_sources})

FILE(GLOB map_system_map_managers_zone_scripts_sources
"${prefix}/Map System/Map Managers/ZoneScripts/*.h"
"${prefix}/Map System/Map Managers/ZoneScripts/*.cpp")
source_group("Map System\\Map Managers\\Zone Scripts" FILES ${map_system_map_managers_zone_scripts_sources})
SET(group_sources ${group_sources} ${map_system_map_managers_zone_scripts_sources})

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

FILE(GLOB unit_system_sources
"${prefix}/Unit Systems/*.h"
"${prefix}/Unit Systems/*.cpp")
source_group("Unit Systems" FILES ${unit_system_sources})
SET(group_sources ${group_sources} ${unit_system_sources})

FILE(GLOB unit_system_player_system_sources
"${prefix}/Unit Systems/Player Systems/*.h"
"${prefix}/Unit Systems/Player Systems/*.cpp")
source_group("Unit Systems\\Player Systems" FILES ${unit_system_player_system_sources})
SET(group_sources ${group_sources} ${unit_system_player_system_sources})

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

FILE(GLOB spell_hooks_sources
"${prefix}/Spell/SpellHooks/*.h"
"${prefix}/Spell/SpellHooks/*.cpp")
source_group("Spell\\Spell Hooks" FILES ${spell_hooks_sources})
SET(group_sources ${group_sources} ${spell_hooks_sources})

FILE(GLOB sources "${prefix}/*.h" "${prefix}/*.cpp"
"${ROOT_PATH}/src/ronin-shared/MemAllocator.*")
source_group("Mainframe\\System" FILES ${sources})

#Remove already added files from our main source files
list(REMOVE_ITEM sources ${group_sources})
#Set the rest of our file sources
SET(SRCS ${SRCS} ${sources} ${group_sources} )
#Set our precompiled header data
include(PCHSupport)
#Set our directories for includes
include_directories( ${GLOBAL_INCLUDE_DIRS} ${RONIN_SOURCE_DIRECTORIES} )

link_directories( ${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH} )
#Add our sources to our project files
add_executable( ${PROJECT_NAME} ${SRCS} )
#Redefine our precompiled header inside our sources
IF( CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} StdAfx.h ${prefix}/StdAfx.cpp)
ENDIF()

#Link our ronin-shared lib.
add_dependencies( ${PROJECT_NAME} ronin-shared zlib )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ronin-shared zlib ${EXTRA_LIBS} )

#Set our resources
set_source_files_properties(Icon.ico resources.rc PROPERTIES LANGUAGE RC)
#Set our compile information
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)
