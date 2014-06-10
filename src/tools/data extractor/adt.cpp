/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#define _CRT_SECURE_NO_DEPRECATE

#include "adt.h"

//
// Adt file loader class
//
ADT_file::ADT_file(const char * filename, HANDLE _handle) : FileLoader(filename, _handle)
{
    a_grid = 0;
}

ADT_file::~ADT_file()
{
}

//
// Adt file check function
//
bool ADT_file::prepareLoadedData()
{
    // Check parent
    if (!FileLoader::prepareLoadedData())
        return false;

    // Check and prepare MHDR
    a_grid = (adt_MHDR *)(buffer+8+version->size);
    if (!a_grid->prepareLoadedData())
        return false;

    //mcnk_offsets
    int ptr = 0;
    int found = 0;
    while (ptr < size)
    {
        if((buffer + ptr)[0] == 'K' &&
            (buffer + ptr)[1] == 'N' &&
            (buffer + ptr)[2] == 'C' &&
            (buffer + ptr)[3] == 'M')
        {
            adt_MCNK * mcnk = (adt_MCNK*)(buffer + ptr);
            assert(mcnk->iy < ADT_CELLS_PER_GRID);
            assert(mcnk->ix < ADT_CELLS_PER_GRID);
            mcnk_offsets[mcnk->iy][mcnk->ix] = mcnk;
            ptr += 4;//go to size
            ptr += 4 + *((uint32*)(buffer + ptr));//skip all datas AND size.
            found ++;
            mcnk->prepareLoadedData();
        }
        else
        {
            ptr += 4;//go to size
            ptr += 4 + *((uint32*)(buffer + ptr)); //skip all datas AND size.
        }
    }
    assert(found == 256);
    return true;
}

bool adt_MHDR::prepareLoadedData()
{
    if (fcc != 'MHDR')
        return false;

    if (size!=sizeof(adt_MHDR)-8)
        return false;

    // Check and prepare MH2O
    if (offsMH2O && !getMH2O()->prepareLoadedData())
        return false;

    return true;
}

bool adt_MCIN::prepareLoadedData()
{
    if (fcc != 'MCIN')
        return false;

    // Check cells data
    for (int i=0; i<ADT_CELLS_PER_GRID;i++)
        for (int j=0; j<ADT_CELLS_PER_GRID;j++)
            if (cells[i][j].offsMCNK && !getMCNK(i,j)->prepareLoadedData())
                return false;

    return true;
}

bool adt_MH2O::prepareLoadedData()
{
    if (fcc != 'MH2O')
        return false;

    // Check liquid data
//  for (int i=0; i<ADT_CELLS_PER_GRID;i++)
//      for (int j=0; j<ADT_CELLS_PER_GRID;j++)

    return true;
}

bool adt_MCNK::prepareLoadedData()
{
    if (fcc != 'MCNK')
        return false;

    // Check height map
    if (offsMCVT && !getMCVT()->prepareLoadedData())
        return false;
    // Check liquid data
    if (offsMCLQ && !getMCLQ()->prepareLoadedData())
        return false;

    return true;
}

bool adt_MCVT::prepareLoadedData()
{
    if (fcc != 'MCVT')
        return false;

    if (size != sizeof(adt_MCVT)-8)
        return false;

    return true;
}

bool adt_MCLQ::prepareLoadedData()
{
    if (fcc != 'MCLQ')
        return false;

    return true;
}