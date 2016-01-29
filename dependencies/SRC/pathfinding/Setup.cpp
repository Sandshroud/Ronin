
#include <g3dlite/G3D.h>
#include <recast/Recast.h>
#include <Detour/Detour.h>
#include <mmaplib/MMapManager.h>

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) { return TRUE; }
#endif

extern "C" __declspec(dllexport) MMapManagerExt *mmap_manager_construction(std::string dataDir, unsigned int mapId)
{
    return new MMapManager(dataDir, mapId);
}
