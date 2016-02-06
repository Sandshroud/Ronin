
#include <string>
#include <iostream>

#include <g3dlite/G3D.h>
#include <vmaplib/VMapLib.h>

extern const char* szWorkDirWmo;
extern const char* szWorkDirTiles;
extern const char* szWorkDirObject;

//=======================================================
bool BuildTiles()
{
    VMAP::TileAssembler* ta = new VMAP::TileAssembler(szWorkDirWmo, szWorkDirTiles, szWorkDirObject);

    bool result = ta->convertWorld2();
    delete ta;
    return result;
}
