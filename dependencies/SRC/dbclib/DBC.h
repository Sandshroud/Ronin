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

    uint32 m_max;
    char *m_stringData, *name, *CFormat;
    std::map<uint32, T*> m_blocks, m_entries;
public:
    DBC()
    {
        m_max = 0;
        memset(&header, 0, sizeof(header));
        name = CFormat = m_stringData = NULL;
    };

    ~DBC()
    {
        if(name) delete name;
        if(CFormat) delete CFormat;
        if(m_stringData) delete m_stringData;
        m_entries.clear();
        while(m_blocks.size())
        {
            T *block = m_blocks.begin()->second;
            m_blocks.erase(m_blocks.begin());
            delete block;
        }
        m_blocks.clear();
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

        uint32 valueMax = header.cols*header.rows;
        if(header.stringsize)
        {
            fseek( f, (valueMax*4), SEEK_CUR );
            m_stringData = new char[header.stringsize];
            fread( m_stringData, header.stringsize, 1, f );
        }

        fseek(f, pos, SEEK_SET);
        return PackData(f);
    }

    bool PackData(FILE *f)
    {
        /* read the data for each row */
        for(uint32 i = 0; i < header.rows; ++i)
        {
            T *block = new T();
            uint32 c = 0, stringCount = 0;
            uint8 *dest_ptr = (uint8*)block;
            const char * t = CFormat;
            size_t len = strlen(CFormat);
            while(*t != 0)
            {
                if((++c) > header.cols)
                {
                    delete block;
                    printf("!!! Read buffer overflow in DBC reading of file %s\n", name);
                    return false;
                }

                switch(*t)
                {
                case 'x': // integer skip
                    {
                        fseek(f, 4, SEEK_CUR);
                    }break;
                case 'p': // Padding, byte skip
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
                            dest_ptr = (uint8*)new_ptr;
                        }
                        else
                        {
                            *((uint32*)dest_ptr) = val;
                            dest_ptr += 4;
                        }
                    }break;
                }
                ++t;
            }

            /* all the time the first field in the dbc is our unique entry */
            uint32 entry = *((uint32*)block);
            if(entry > m_max) m_max = entry;
            m_blocks.insert(std::make_pair(i, block));
            m_entries.insert(std::make_pair(entry, block));
        }

        std::string file_name(name);
        if(file_name.find_last_of('/') != std::string::npos)
            file_name = file_name.substr(file_name.find_last_of('/')+1, file_name.size());
        sLog.Notice("DBC", "Loaded %s (%u rows)", file_name.c_str(), header.rows);
        return true;
    }

    uint32 GetNumRows() { return header.rows; }
    uint32 GetMaxEntry() { return m_max; }

    T *LookupEntryTest(uint32 index)
    {
        if(index > m_max || m_entries.find(index) == m_entries.end())
        {
            sLog.Error("DBC", "LookupTest for %s failed on %u", name, index);
            return NULL;
        }
        return m_entries.at(index);
    }

    T *LookupEntry(uint32 index)
    {
        if(index > m_max || m_entries.find(index) == m_entries.end())
            return NULL;
        return m_entries.at(index);
    }

    T *LookupRow(uint32 index)
    {
        if(index >= header.rows)
            return NULL;
        return m_blocks[index];
    }
};
