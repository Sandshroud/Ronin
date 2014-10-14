/***
 * Demonstrike Core
 */

#pragma once

#include <cassert>
#include <string>
#if PLATFORM != PLATFORM_WIN
#include <string.h>
#include <cstdlib>
#endif

class DBCFile
{
public:
    DBCFile();
    ~DBCFile();

    // Open database. It must be openened before it can be used.
    bool open(const char*);
    bool openFromMPQ(HANDLE);

    // Dump the 
    bool DumpBufferToFile(const char*fn);

    // Database exceptions
    class Exception
    {
    public:
        Exception(const std::string &message): message(message)
        { }
        virtual ~Exception()
        { }
        const std::string &getMessage() {return message;}
    private:
        std::string message;
    };
    class NotFound: public Exception
    {
    public:
        NotFound(): Exception("Key was not found")
        { }
    };
    // Iteration over database
    class Iterator;
    class Record
    {
    public:
        float getFloat(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<float*>(offset+field*4);
        }
        unsigned int getUInt(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<unsigned int*>(offset+field*4);
        }
        int getInt(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<int*>(offset+field*4);
        }
        const char *getString(size_t field) const
        {
            assert(field < file.fieldCount);
            size_t stringOffset = getUInt(field);
            assert(stringOffset < file.stringSize);
            return reinterpret_cast<char*>(file.stringTable + stringOffset);
        }
        //used by external tool
        const void *getRowStart() const
        {
            return reinterpret_cast<void*>(offset);
        }
        void SetFloat(size_t field,float value) const
        {
            assert(field < file.fieldCount);
            *reinterpret_cast<float*>(offset+field*4) = value;
        }
        void SetUInt(size_t field,unsigned int value) const
        {
            assert(field < file.fieldCount);
            *reinterpret_cast<unsigned int*>(offset+field*4) = value;
        }
        void SetInt(size_t field,signed int value) const
        {
            assert(field < file.fieldCount);
            *reinterpret_cast<int*>(offset+field*4) =  value;
        }
        void SetString(size_t field,const char *value) const
        {
            assert(field < file.fieldCount);
            size_t stringOffset = getUInt(field);
            assert(stringOffset < file.stringSize);
            strcpy(reinterpret_cast<char*>(file.stringTable + stringOffset),value);
        }

        Record& operator=(Record& src)
        {
            this->file = src.file;
            this->offset = src.offset;
            return *this;
        }

    private:
        Record(DBCFile &file, unsigned char *offset): file(file), offset(offset) {}
        DBCFile &file;
        unsigned char *offset;

        friend class DBCFile;
        friend class DBCFile::Iterator;
    };
    /** Iterator that iterates over records
    */
    class Iterator
    {
    public:
        Iterator(DBCFile *file, size_t offset = 0) : m_dbcFile(file), m_offset(offset) {}
        Iterator & operator++() { m_offset += m_dbcFile->recordSize; return *this; }
        Record getRecord() { return Record(*m_dbcFile, m_dbcFile->data+m_offset); }
        Record operator*() const { return Record(*m_dbcFile, m_dbcFile->data+m_offset); }
        const Record* operator->() const { return &Record(*m_dbcFile, m_dbcFile->data+m_offset); }
        /// Comparison
        bool operator==(const Iterator &b) const { return m_offset == b.m_offset; }
        bool operator!=(const Iterator &b) const { return m_offset != b.m_offset; }
    private:
        DBCFile *m_dbcFile;
        size_t m_offset;
    };

    // Get record by id
    Record getRecord(size_t id);
    /// Get begin iterator over records
    Iterator begin();
    /// Get begin iterator over records
    Iterator end();
    /// Trivial
    size_t getRecordCount() const { return recordCount;}
    size_t getFieldCount() const { return fieldCount; }
    size_t getMaxId() const { return maxId; }
////////////////////////////////////////////////////////////////////////////////////////////
    int AddRecord(); //simply add an empty record to the end of the data section
    int AddString(const char *new_string); //simply add an empty record to the end of the string section
    int GetRecordSize() { return recordSize; }
////////////////////////////////////////////////////////////////////////////////////////////
private:
    void CalcMaxId();
    std::string filename;
    unsigned int recordSize;
    unsigned int recordCount;
    unsigned int fieldCount;
    unsigned int stringSize;
    unsigned int maxId;
    unsigned char *data;
    unsigned char *stringTable;
    char header[4];
};
