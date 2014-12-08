/***
 * Demonstrike Core
 */

#pragma once

template<class T> class DBC
{
    struct
    {
        // "WDBC"
        char title[4];

        // Our old WDBC data we can re-use
//      struct
//      {
            uint32 rows;
            uint32 cols;
            uint32 fieldcount;
            uint32 stringsize;
//      }structureInfo;
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
    DBC()
    {
        memset(&header, 0, sizeof(header));
        Loaded = false;
        Blocks = NULL;
        Entries = NULL;
        m_firstEntry = NULL;
        m_max = 0;
        m_int32Values = NULL;
        m_floatValues = NULL;
        m_stringData = NULL;
    };

    ~DBC()
    {
        if(m_int32Values)
            delete [] m_int32Values;
        if(m_floatValues)
            delete [] m_floatValues;
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
            sLog.Error("DBC", "DBC %s doesn't exist!\n", filename);
            return false;
        }

        if(fread(header.title, 4, 1, f) != 1)
        {
            fclose(f);
            sLog.Error("DBC", "DBC %s has incorrect header size!\n", filename);
            return false;
        }

        if(strcmp(header.title, "WDBC"))
        {
            fclose(f);
            sLog.Error("DBC", "DBC %s has incorrect header %s!\n", filename, header.title);
            return false;
        }

        if((fread(&header.rows, 4, 1, f) != 1) || (fread(&header.cols, 4, 1, f) != 1)
             || (fread(&header.fieldcount, 4, 1, f) != 1) || (fread(&header.stringsize, 4, 1, f) != 1))
        {
            fclose(f);
            sLog.Error("DBC", "DBC %s doesn't contain the correct structure info!\n");
            return false;
        }

        size_t formatLen = strlen(CFormat);
        if(formatLen != header.cols)
        {
            fclose(f);
            sLog.Error("DBC", "DBC %s has an incorrect format!(%u/%u)\n", filename, formatLen, header.cols);
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
            uint32 c = 0;
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

                switch(*t)
                {
                case 'x': // Byte skip
                    {
                        fseek(f, 4, SEEK_CUR);
                    }break;
                case 'p': // Padding, bit skip
                    {
                        fseek(f, 1, SEEK_CUR);
                    }break;
                case 'b':
                    {
                        uint8 val;
                        fread(&val, 1, 1, f);
                        *dest_ptr = val;
                        dest_ptr++;
                    }break;
                default:
                    {
                        uint32 val;
                        fread(&val, 4, 1, f);
                        if(*t == 's')
                        {
                            static const char * null_str = "";
                            char ** new_ptr = (char**)dest_ptr, *ptr;
                            if( val < header.stringsize)
                                ptr = m_stringData + val;
                            else ptr = (char*)null_str;
                            *new_ptr = ptr; new_ptr++;
                            dest_ptr = (uint32*)new_ptr;
                        }
                        else
                        {
                            *dest_ptr = val;
                            dest_ptr++;
                        }
                    }break;
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

        sLog.Notice("DBC", "Loaded %s (%u rows)", name, header.rows);
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
            sLog.Error("DBC", "LookupTest for %s failed on %u", name, index);
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
