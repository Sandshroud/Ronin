#include "dbcfile.h"

DBCFile::DBCFile(const std::string &filename):
    filename(filename),
    data(0)
{
    recordCount = 0;
}

void DBCFile::open()
{
    FILE* f = fopen(filename.c_str(), "r");
    if(f == NULL)
        return;

    char header[4];
    unsigned int na,nb,es,ss;

    fread(header, 4, 1, f); // Number of records
    assert(header[0]=='W' && header[1]=='D' && header[2]=='B' && header[3] == 'C');
    fread(&na, 4, 1, f); // Number of records
    fread(&nb, 4, 1, f); // Number of fields
    fread(&es, 4, 1, f); // Size of a record
    fread(&ss, 4, 1, f); // String size

    recordSize = es;
    recordCount = na;
    fieldCount = nb;
    stringSize = ss;
    assert(fieldCount*4 == recordSize);

    data = new unsigned char[recordSize*recordCount+stringSize];
    stringTable = data + recordSize*recordCount;
    fread(data, recordSize*recordCount+stringSize, 1, f);
    fclose(f);
}

DBCFile::~DBCFile()
{
    delete [] data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

DBCFile::Iterator DBCFile::begin()
{
    assert(data);
    return Iterator(*this, data);
}
DBCFile::Iterator DBCFile::end()
{
    assert(data);
    return Iterator(*this, stringTable);
}

size_t DBCFile::getMaxId()
{
    assert(data);

    size_t maxId = 0;
    for(size_t i = 0; i < getRecordCount(); ++i)
    {
        if(maxId < getRecord(i).getUInt(0))
            maxId = getRecord(i).getUInt(0);
    }
    return maxId;
}
