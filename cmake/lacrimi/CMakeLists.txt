project(Lacrimi)

SET( prefix ${ROOT_PATH}/src/Lacrimi/Source )

#Lacrimi files
FILE(GLOB lacrimi
"${prefix}/*.h"
"${prefix}/*.cpp"
"${prefix}/Setup/*.h"
"${prefix}/Setup/*.cpp"
"${prefix}/PrecompiledHeaders/*.h"
"${prefix}/PrecompiledHeaders/*.cpp")
source_group("Lacrimi" FILES ${lacrimi})

FILE(GLOB dependencies
"${prefix}/Dependencies/*.h"
"${prefix}/Dependencies/*.cpp")
source_group("Dependencies" FILES ${dependencies})

FILE(GLOB lua_handler
"${prefix}/LuaHandler/*.h"
"${prefix}/LuaHandler/*.cpp")
source_group("LuaHandler" FILES ${lua_handler})

FILE(GLOB lua_functions
"${prefix}/LuaHandler/Functions/*.h"
"${prefix}/LuaHandler/Functions/*.cpp")
source_group("LuaHandler\\LuaFunctions" FILES ${lua_functions})

FILE(GLOB scripts
"${prefix}/Scripts/*.h"
"${prefix}/Scripts/*.cpp")
source_group("Scripts" FILES ${scripts})

FILE(GLOB scripts_spells
"${prefix}/Scripts/SpellScripts/*.h"
"${prefix}/Scripts/SpellScripts/*.cpp")
source_group("Scripts\\SpellScripts" FILES ${scripts_spells})

SET( SRCS ${SRCS} ${lacrimi} )
SET( SRCS ${SRCS} ${dependencies} )
SET( SRCS ${SRCS} ${lua_handler} )
SET( SRCS ${SRCS} ${lua_functions} )
SET( SRCS ${SRCS} ${scripts} )
SET( SRCS ${SRCS} ${scripts_spells} )

#Scripts - Level 0
FILE(GLOB scripts_eastern_kingdoms
"${prefix}/Scripts/Level 0/Eastern Kingdoms/*.h"
"${prefix}/Scripts/Level 0/Eastern Kingdoms/*.cpp")
source_group("Scripts\\Level 0\\Eastern Kingdoms" FILES ${scripts_eastern_kingdoms})

FILE(GLOB scripts_eastern_kingdoms_cities
"${prefix}/Scripts/Level 0/Eastern Kingdoms/Cities/*.h"
"${prefix}/Scripts/Level 0/Eastern Kingdoms/Cities/*.cpp")
source_group("Scripts\\Level 0\\Eastern Kingdoms\\Cities" FILES ${scripts_eastern_kingdoms_cities})

FILE(GLOB scripts_kalimdor
"${prefix}/Scripts/Level 0/Kalimdor/*.h"
"${prefix}/Scripts/Level 0/Kalimdor/*.cpp")
source_group("Scripts\\Level 0\\Kalimdor" FILES ${scripts_kalimdor})

FILE(GLOB scripts_kalimdor_cities
"${prefix}/Scripts/Level 0/Kalimdor/Cities/*.h"
"${prefix}/Scripts/Level 0/Kalimdor/Cities/*.cpp")
source_group("Scripts\\Level 0\\Kalimdor\\Cities" FILES ${scripts_kalimdor_cities})

FILE(GLOB scripts_outlands
"${prefix}/Scripts/Level 0/Outlands/*.h"
"${prefix}/Scripts/Level 0/Outlands/*.cpp")
source_group("Scripts\\Level 0\\Outlands" FILES ${scripts_outlands})

FILE(GLOB scripts_outlands_cities
"${prefix}/Scripts/Level 0/Outlands/Cities/*.h"
"${prefix}/Scripts/Level 0/Outlands/Cities/*.cpp")
source_group("Scripts\\Level 0\\Outlands\\Cities" FILES ${scripts_outlands_cities})

FILE(GLOB scripts_northrend
"${prefix}/Scripts/Level 0/Northrend/*.h"
"${prefix}/Scripts/Level 0/Northrend/*.cpp")
source_group("Scripts\\Level 0\\Northrend" FILES ${scripts_northrend})

FILE(GLOB scripts_northrend_cities
"${prefix}/Scripts/Level 0/Northrend/Cities/*.h"
"${prefix}/Scripts/Level 0/Northrend/Cities/*.cpp")
source_group("Scripts\\Level 0\\Northrend\\Cities" FILES ${scripts_northrend_cities})

FILE(GLOB scripts_northrend_pvp
"${prefix}/Scripts/Level 0/Northrend/PvP/*.h"
"${prefix}/Scripts/Level 0/Northrend/PvP/*.cpp")
source_group("Scripts\\Level 0\\Northrend\\PvP" FILES ${scripts_northrend_pvp})

FILE(GLOB scripts_ebon_hold
"${prefix}/Scripts/Level 0/Ebon Hold/*.h"
"${prefix}/Scripts/Level 0/Ebon Hold/*.cpp")
source_group("Scripts\\Level 0\\Ebon Hold" FILES ${scripts_ebon_hold})

SET( SRCS ${SRCS} ${scripts_eastern_kingdoms} )
SET( SRCS ${SRCS} ${scripts_eastern_kingdoms_cities} )
SET( SRCS ${SRCS} ${scripts_kalimdor} )
SET( SRCS ${SRCS} ${scripts_kalimdor_cities} )
SET( SRCS ${SRCS} ${scripts_outlands} )
SET( SRCS ${SRCS} ${scripts_outlands_cities} )
SET( SRCS ${SRCS} ${scripts_northrend} )
SET( SRCS ${SRCS} ${scripts_northrend_cities} )
SET( SRCS ${SRCS} ${scripts_northrend_pvp} )
SET( SRCS ${SRCS} ${scripts_ebon_hold} )

#Scripts - Level 2
FILE(GLOB scripts_blackfathom_deeps
"${prefix}/Scripts/Level 2/Blackfathom Deeps/*.h"
"${prefix}/Scripts/Level 2/Blackfathom Deeps/*.cpp")
source_group("Scripts\\Level 2\\Blackfathom Deeps" FILES ${scripts_blackfathom_deeps})

SET( SRCS ${SRCS} ${scripts_blackfathom_deeps} )

#Add our dependencies and our ScriptLib declaration
link_directories( ${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH} )
add_definitions(-DSCRIPTLIB)

#Add our source files
add_library( ${PROJECT_NAME} SHARED ${SRCS} )

#Set up our precompiled header system
include(PCHSupport)
include_directories( ${GLOBAL_INCLUDE_DIRS} ${HEARTHSTONE_SOURCE_DIRECTORIES} ${prefix} ${prefix}/PrecompiledHeaders )
IF( CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} LacrimiStdAfx.h ${prefix}/PrecompiledHeaders/LacrimiStdAfx.cpp)
ENDIF()

#Link our hearthstone-shared lib.
add_dependencies( ${PROJECT_NAME} hearthstone-world consolelog lualib )
target_link_libraries( ${PROJECT_NAME} hearthstone-world consolelog lualib ${EXTRA_LIBS} )
