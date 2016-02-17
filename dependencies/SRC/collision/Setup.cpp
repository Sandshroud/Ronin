
#include <g3dlite/G3D.h>
#include <vmapLib/VMapLib.h>

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) { return TRUE; }
#endif

extern "C" __declspec(dllexport) VMAP::VMapManagerExt *vmap_manager_construction(std::string vmapObjDir)
{
    return new VMAP::VMapManager(vmapObjDir);
}
