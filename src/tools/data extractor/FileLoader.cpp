/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#define _CRT_SECURE_NO_DEPRECATE

#include "FileLoader.h"

bool FileLoader::prepareLoadedData()
{
    // Check version
    version = (file_MVER *) buffer;
    if (version->fcc != 'MVER')
        return false;
    if (version->ver != FILE_FORMAT_VERSION)
        return false;
    return true;
}
