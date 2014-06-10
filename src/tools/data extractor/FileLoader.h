/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#ifndef FILELOADER_H
#define FILELOADER_H

#include "headers.h"
#include "typedefs.h"
#include "MPQFile.h"

#define FILE_FORMAT_VERSION 18

//
// File version chunk
//
struct file_MVER
{
    union
    {
        uint32 fcc;
        char   fcc_txt[4];
    };
    uint32 size;
    uint32 ver;
};

class FileLoader : public MPQFile
{
public:
    FileLoader(const char* filename, HANDLE handle): MPQFile(filename, handle) {};

    virtual bool prepareLoadedData();

    file_MVER *version;
};

#endif