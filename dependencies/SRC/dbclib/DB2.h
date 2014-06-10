/***
 * Demonstrike Core
 */

#pragma once

template<class T> class DB2
{
    struct
    {
        // "WDB2" - or: 0x32424457
        char title[4];

        // Our old WDBC data we can re-use
//      struct
//      {
            uint32 rows;
            uint32 cols;
            uint32 fieldcount;
            uint32 stringsize;
//      }structureInfo;

        // Our new WDB2 data, since it's an extended DBC file.
//      struct
//      {
            uint32 tableHash;
            uint32 build;
            uint32 unk1;
//      }DB2Data;

        // if build > 12880 then we have extended data to use.
//      struct
//      {
            uint32 ChunkSize;
            uint32 ChunkLength;
            uint32 locale;
            uint32 unk3;
//      }ExtendedDB2Data;
    }header;

    uint32 ValueMax;
    union
    {
        int32  *m_int32Values;
        float *m_floatValues;
    };

    char *m_stringData;
    const char* name;
    bool Loaded;
    uint32 m_max;

    char *CFormat;
    T *m_firstEntry;
    T *Blocks;
    T **Entries;
public:
    DB2()
    {
        memset(&header, 0, sizeof(header));
        Loaded = false;
        Blocks = NULL;
        Entries = NULL;
        m_firstEntry = NULL;
        m_max = 0;
        m_stringData = NULL;
    };

    ~DB2()
    {
        if(m_int32Values)
            delete [] m_int32Values;
        if(m_stringData)
            delete [] m_stringData;
        if(Blocks)
            free(Blocks);
        if(Entries)
            free(Entries);
    }

    bool Load(const char *filename, const char* format)
    {
        name = strdup(filename);
        CFormat = strdup(format);
        FILE *f = fopen(filename, "rb");
        if(!f)
        {
            bLog.Error("DB2", "DB2 %s doesn't exist!\n", filename);
            return false;
        }

        if(fread(header.title, 4, 1, f) != 1)
        {
            fclose(f);
            bLog.Error("DB2", "DB2 %s has incorrect header size!\n", filename);
            return false;
        }

        if(strcmp(header.title, "WDB2"))
        {
            fclose(f);
            bLog.Error("DB2", "DB2 %s has incorrect header %s!\n", filename, header.title);
            return false;
        }

        if((fread(&header.rows, 4, 1, f) != 1) || (fread(&header.cols, 4, 1, f) != 1)
             || (fread(&header.fieldcount, 4, 1, f) != 1) || (fread(&header.stringsize, 4, 1, f) != 1))
        {
            fclose(f);
            bLog.Error("DB2", "DB2 %s doesn't contain the correct structure info!\n", filename, header.title);
            return false;
        }

        if((fread(&header.tableHash, 4, 1, f) != 1) || (fread(&header.build, 4, 1, f) != 1)
             || (fread(&header.unk1, 4, 1, f) != 1))
        {
            fclose(f);
            bLog.Error("DB2", "DB2 %s doesn't contain the correct db2 data!\n", filename, header.title);
            return false;
        }

        // Check for extended data.
        if(header.build > 12880)
        {   // We have extended data in these DB2, so read them.
            if((fread(&header.ChunkSize, 4, 1, f) != 1) || (fread(&header.ChunkLength, 4, 1, f) != 1)
                 || (fread(&header.locale, 4, 1, f) != 1) || (fread(&header.unk3, 4, 1, f) != 1))
            {
                fclose(f);
                bLog.Error("DB2", "DB2 %s doesn't contain the extended db2 data like it should!\n", filename, header.title);
                return false;
            }

            if (header.ChunkLength != 0)
            {
                int32 diff = header.ChunkLength-header.ChunkSize+1;
                fseek(f, diff*4+diff*2, SEEK_CUR);
            }
        }

        if(strlen(CFormat) != header.cols)
        {
            fclose(f);
            bLog.Error("DBC", "DBC %s has an incorrect format!\n", filename);
            return false;
        }
        uint32 pos = ftell(f);

        ValueMax = header.cols*header.rows;
        if(header.stringsize)
        {
            fseek( f, (ValueMax*4), SEEK_CUR );
            m_stringData = (char*)malloc(header.stringsize);
            fread( m_stringData, header.stringsize, 1, f );
        }

        fseek(f, pos, SEEK_SET);
        Loaded = true;
        return PackData(f);
    }

    bool PackData(FILE *f)
    {
        Blocks = (T*)malloc(header.rows * sizeof(T));;
        /* read the data for each row */
        for(uint32 i = 0; i < header.rows; ++i)
        {
            uint32 c = 0, val;
            uint32 stringCount = 0;
            memset(&Blocks[i], 0, sizeof(T));
            uint32 *dest_ptr = (uint32*)&Blocks[i];
            const char * t = CFormat;
            size_t len = strlen(CFormat);
            while(*t != 0)
            {
                if((++c) > header.cols)
                {
                    ++t;
                    printf("!!! Read buffer overflow in DBC reading of file %s\n", name);
                    continue;
                }

                fread(&val, 4, 1, f);
                if(*t == 'x')
                {
                    ++t;
                    continue;        // skip!
                }
                if(*t == 's')
                {
                    char ** new_ptr = (char**)dest_ptr;
                    static const char * null_str = "";
                    char * ptr;
                    if( val < header.stringsize)
                        ptr = m_stringData + val;
                    else
                        ptr = (char*)null_str;

                    *new_ptr = ptr;
                    new_ptr++;
                    dest_ptr = (uint32*)new_ptr;
                }
                else
                {
                    *dest_ptr = val;
                    dest_ptr++;
                }

                ++t;
            }

            if(m_firstEntry == NULL)
                m_firstEntry = &Blocks[i];
            /* all the time the first field in the dbc is our unique entry */
            if(*(uint32*)&Blocks[i] > m_max)
                m_max = *(uint32*)&Blocks[i];
        }

        Entries = (T**)malloc(sizeof(T*) * (m_max+1));
        ASSERT(Entries);

        memset(Entries, 0, (sizeof(T*) * (m_max+1)));
        for(uint32 i = 0; i < header.rows; ++i)
            Entries[*(uint32*)&Blocks[i]] = &Blocks[i];

        bLog.Notice("DB2", "Loaded %s (%u rows build %u)", name, header.rows, header.build);
        return true;
    }

    uint32 GetNumRows()
    {
        return header.rows;
    };

    uint32 GetMaxRow()
    {
        return m_max;
    }

    void SetRow(uint32 i, T * t)
    {
        if(i < m_max)
            Entries[i] = t;
    }

    T *LookupEntryTest(uint32 index)
    {
        if(index > m_max || Entries[index] == NULL)
        {
            bLog.Error("DBC", "LookupTest for %s failed on %u", name, index);
            return NULL;
        }
        return Entries[index];
    }

    T *LookupEntry(uint32 index)
    {
        if(index > m_max || Entries[index] == NULL)
            return NULL;
        return Entries[index];
    }

    T *LookupRow(uint32 index)
    {
        if(index >= header.rows)
            return NULL;
        else
            return &Blocks[index];
    }

    T *GetFirstBlock()
    {
        return &Blocks[0];
    }

    T *GetLastBlock()
    {
        return &Blocks[header.rows];
    }
};
