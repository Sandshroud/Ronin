/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#include "headers.h"
#include "misc.h"
#include "MPQ.h"

bool GetMPQHandle(const char* file, HANDLE &mpqhandle)
{
    HANDLE MPQ = NULL;
    for(int8 p = 0; p < 4; p++)
    {
        MPQ = DataMPQs[p];
        if(SFileHasFile(MPQ, file))
            break;
        MPQ = NULL;
    }
    if(MPQ == NULL)
        return false;
    mpqhandle = MPQ;
    return true;
}

bool FileExists( const char* FileName )
{
    FILE *fp;
    fopen_s(&fp, FileName,"r");
    if( fp )
    {
        fclose(fp);
        return true;
    }
    return false;
}

void CreateDir( const std::string& Path )
{
#ifdef _WIN32
    _mkdir(Path.c_str());
#else
    mkdir(Path.c_str(), 0777);
#endif
}

int ReadBuild(int locale)
{
    // include build info file also
    std::string filename  = std::string("component.wow-")+langs[locale]+".txt";
    //printf("Read %s file... ", filename.c_str());

    CreateDir("./dbc/");
    ExtractFileToHardDrive(localeMPQ[0], filename.c_str(), (std::string("./dbc/") + filename).c_str());

    std::string text;
    std::string temp;

    std::ifstream fichier((std::string("./dbc/") + filename).c_str(), std::ios::in);
    if(!fichier)
        assert(false && "Error when loading component.wow-...");
    while(fichier)
    {
        fichier >> temp;
        text += temp;
    }

    size_t pos = text.find("version=\"");
    size_t pos1 = pos + strlen("version=\"");
    size_t pos2 = text.find("\"",pos1);
    if (pos == text.npos || pos2 == text.npos || pos1 >= pos2)
    {
        printf("Fatal error: Invalid  %s file format!\n", filename.c_str());
        exit(1);
    }

    std::string build_str = text.substr(pos1,pos2-pos1);

    int build = atoi(build_str.c_str());
    if (build <= 0)
    {
        printf("Fatal error: Invalid  %s file format!\n", filename.c_str());
        exit(1);
    }

    return build;
}