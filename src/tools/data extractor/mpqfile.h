
#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define __INCLUDE_CRYPTOGRAPHY__
#define __STORMLIB_SELF__

#include <map>
#include <vector>
#include <list>
#include <errno.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <map>
#include <fstream>
#include <iostream>

#include <SharedDependencyDefines.h>

#include <StormLib/StormLib.h>
#include <StormLib/StormCommon.h>

#define _CRT_SECURE_NO_DEPRECATE
#include <cstdio>
#include <iostream>
#include <string>

#ifdef WIN32
    #include <Windows.h>
    #include <sys/stat.h>
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <sys/stat.h>
    #define ERROR_PATH_NOT_FOUND ERROR_FILE_NOT_FOUND
#endif

using namespace std;

class MPQFile
{
    //MPQHANDLE handle;
    bool eof;
    char *buffer;
    size_t pointer,size;

    // disable copying
    MPQFile(const MPQFile &f);
    void operator=(const MPQFile &f);

public:
    MPQFile(HANDLE mpq, const char* filename, bool warnNoExist = true);    // filenames are not case sensitive
    ~MPQFile() { close(); }
    size_t read(void* dest, size_t bytes);
    size_t getSize() { return size; }
    size_t getPos() { return pointer; }
    char* getBuffer() { return buffer; }
    char* getPointer() { return buffer + pointer; }
    bool isEof() { return eof; }
    void seek(int offset);
    void seekRelative(int offset);
    void close();
};

inline void flipcc(char *fcc)
{
    char t;
    t=fcc[0];
    fcc[0]=fcc[3];
    fcc[3]=t;
    t=fcc[1];
    fcc[1]=fcc[2];
    fcc[2]=t;
}
