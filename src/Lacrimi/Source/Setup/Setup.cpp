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

#include "LacrimiStdAfx.h"
#define SKIP_ALLOCATOR_SHARING 1
#include <ScriptInterface/ScriptSetup.h>

extern "C" SCRIPT_DECL uint32 _exp_get_script_type()
{
    return 42; // <Troll Face>
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
    sWorld.LacrimiPtr = new Lacrimi(mgr);
    // Set up our scripts in this thread.
    sWorld.LacrimiPtr->SetupScripts();

    // Execute our run thread here
    sWorld.LacrimiThread = ((ThreadContext*)sWorld.LacrimiPtr);
    ThreadPool.ExecuteTask("Lacrimi", sWorld.LacrimiPtr);
}

#ifdef WIN32

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}

#endif
