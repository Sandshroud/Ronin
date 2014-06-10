/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#include "headers.h"
#include "MPQFile.h"
#include "MPQ.h"

MPQFile::MPQFile(const char* filename, HANDLE handle_)
{
    handle = handle_;
    size = 0;
    eof = true;
    buffer = NULL;
    pointer = 0;

    buffer = ExtractFileToMemory(handle_, filename, size);
    if(buffer == NULL || size == 0)
        eof = true;
    else
        eof = false;
}

size_t MPQFile::read(void* dest, size_t bytes)
{
    size_t i = 0;
    for(; i < bytes && pointer < size; i++)
        ((char*)dest)[i] = *(buffer + (pointer++));
    return i;
}

void MPQFile::seek(int offset)
{
    pointer = offset;
    eof = pointer >= size;
}

void MPQFile::seekRelative(int offset)
{
    pointer += offset;
    eof = pointer >= size;
}

void MPQFile::close()
{
    delete[] buffer;
    handle = NULL;
    eof = true;
    buffer = NULL;
    pointer = 0;
    size = 0;
}