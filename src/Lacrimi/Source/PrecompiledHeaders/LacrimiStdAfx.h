/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#ifndef LACRIMI_PCH
#define LACRIMI_PCH

#include "StdAfx.h"
#include "../Lacrimi.h"
#include "../Setup/Setup.h"

// Lua Engine
#include "../LuaHandler/LUADefines.h"
#include "../LuaHandler/LUAEngine.h"
#include "../LuaHandler/LUAObjects.h"
#include "../LuaHandler/LUAEventMgr.h"
#include "../LuaHandler/LUAEngineMgr.h"
#include "../LuaHandler/LUALunar.h"
#include "../LuaHandler/LUAGUIDMgr.h"
#include "../LuaHandler/Functions/FunctionSpell.h"
#include "../LuaHandler/Functions/FunctionGameObjects.h"
#include "../LuaHandler/Functions/FunctionGlobal.h"
#include "../LuaHandler/Functions/FunctionItems.h"
#include "../LuaHandler/Functions/FunctionPacket.h"
#include "../LuaHandler/Functions/FunctionAura.h"
#include "../LuaHandler/Functions/FunctionTaxi.h"
#include "../LuaHandler/Functions/FunctionUnits.h"
#include "../LuaHandler/Functions/LuaSqlApi.h"
#include "../LuaHandler/Functions/TableFunctions.h"

// Dependencies
#include "../Dependencies/Functions.h"
#include "../Dependencies/Base.h"

// Scripts
#include "../Scripts/ScriptHeaders.h"

#endif // LACRIMI_PCH