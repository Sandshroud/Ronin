/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#ifndef MPQFILE_H
#define MPQFILE_H

#include "headers.h"

class MPQFile
{
protected:
    HANDLE handle;
    bool eof;
    char *buffer;
    int pointer;
    int size;

    // disable copying
    MPQFile(const MPQFile &f) {}
    void operator=(const MPQFile &f) {}

public:
    MPQFile(const char* filename, HANDLE handle);   // filenames are not case sensitive
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

#endif