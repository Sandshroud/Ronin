/*
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
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

    uint32 m_max;
    char *m_stringData, *name, *CFormat;
    std::map<uint32, T*> m_blocks, m_entries;
public:
    DB2()
    {
        m_max = 0;
        memset(&header, 0, sizeof(header));
        name = CFormat = m_stringData = NULL;
    };

    ~DB2()
    {
        if(name) delete name;
        if(CFormat) delete CFormat;
        if(m_stringData) delete m_stringData;
        name = CFormat = m_stringData = NULL;
        m_entries.clear();
        for(std::map<uint32, T*>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++)
            delete itr->second;
        m_blocks.clear();
    }

    bool Load(const char *filename, const char* format)
    {
        name = strdup(filename);
        CFormat = strdup(format);
        FILE *f = fopen(filename, "rb");
        if(!f)
        {
            sLog.Error("DB2", "DB2 %s doesn't exist!\n", filename);
            return false;
        }

        if(fread(header.title, 4, 1, f) != 1)
        {
            fclose(f);
            sLog.Error("DB2", "DB2 %s has incorrect header size!\n", filename);
            return false;
        }

        if(strcmp(header.title, "WDB2"))
        {
            fclose(f);
            sLog.Error("DB2", "DB2 %s has incorrect header %s!\n", filename, header.title);
            return false;
        }

        if((fread(&header.rows, 4, 1, f) != 1) || (fread(&header.cols, 4, 1, f) != 1)
             || (fread(&header.fieldcount, 4, 1, f) != 1) || (fread(&header.stringsize, 4, 1, f) != 1))
        {
            fclose(f);
            sLog.Error("DB2", "DB2 %s doesn't contain the correct structure info!\n", filename, header.title);
            return false;
        }

        if((fread(&header.tableHash, 4, 1, f) != 1) || (fread(&header.build, 4, 1, f) != 1)
             || (fread(&header.unk1, 4, 1, f) != 1))
        {
            fclose(f);
            sLog.Error("DB2", "DB2 %s doesn't contain the correct db2 data!\n", filename, header.title);
            return false;
        }

        // Check for extended data.
        if(header.build > 12880)
        {   // We have extended data in these DB2, so read them.
            if((fread(&header.ChunkSize, 4, 1, f) != 1) || (fread(&header.ChunkLength, 4, 1, f) != 1)
                 || (fread(&header.locale, 4, 1, f) != 1) || (fread(&header.unk3, 4, 1, f) != 1))
            {
                fclose(f);
                sLog.Error("DB2", "DB2 %s doesn't contain the extended db2 data like it should!\n", filename, header.title);
                return false;
            }

            if (header.ChunkLength != 0)
            {
                int32 diff = header.ChunkLength-header.ChunkSize+1;
                fseek(f, diff*4+diff*2, SEEK_CUR);
            }
        }

        size_t formatLen = strlen(CFormat);
        if(formatLen != header.cols)
        {
            fclose(f);
            sLog.Error("DB2", "DB2 %s has an incorrect format!(%u/%u)\n", filename, formatLen, header.cols);
            return false;
        }
        uint32 pos = ftell(f);
        if(header.stringsize)
        {
            fseek( f, (header.cols*header.rows*4), SEEK_CUR );
            m_stringData = (char*)malloc(header.stringsize);
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
            memset(block, 0, sizeof(T));

            uint32 c = 0, stringCount = 0;
            uint8 *dest_ptr = (uint8*)block;
            const char * t = CFormat;
            size_t len = strlen(CFormat);
            while(*t != 0)
            {
                if((++c) > header.cols)
                {
                    printf("!!! Read buffer overflow in DBC reading of file %s\n", name);
                    delete block;
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
        sLog.Notice("DB2", "Loaded %s (%u rows build %u)", file_name.c_str(), header.rows, header.build);
        return true;
    }

    uint32 GetNumRows() { return header.rows; }
    uint32 GetMaxRow() { return m_max; }

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
