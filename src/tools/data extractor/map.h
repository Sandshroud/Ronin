/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#ifndef MAP_H
#define MAP_H

#include "typedefs.h"

void ExtractMapsFromMpq(uint32 build);

void CleanCache();
bool StoreADTData(HANDLE mpqarchive, char *filename, int cell_y, int cell_x, uint32 build);
bool ConvertADT(int cell_x, int cell_y, uint32 build, FILE* out_file);

#endif