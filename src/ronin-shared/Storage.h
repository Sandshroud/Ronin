/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

// Storage max based off guid entry max
static unsigned long STORAGE_ARRAY_MAX = 0x00FFFFFF;

/** Base iterator class, returned by MakeIterator() functions.
 */
template<class T> class SERVER_DECL StorageContainerIterator
{
protected:
    /** Currently referenced object
     */
    T * Pointer;
public:
    virtual ~StorageContainerIterator() {}

    /** Returns the currently stored object
     */
    RONIN_INLINE T * Get() { return Pointer; }

    /** Sets the current object to P
     */
    RONIN_INLINE void Set(T * P) { Pointer = P; }

    /** Are we at the end of the storage container?
     */
    RONIN_INLINE bool AtEnd() { return (Pointer == 0); }

    /** Virtual function to increment to the next element
     */
    virtual bool Inc() = 0;

    /** Virtual function to destroy the iterator
     */
    virtual void Destruct() = 0;
};

template<class T> class SERVER_DECL HashMapStorageContainer
{
public:
    typename std::map<uint32, T*> _map;

    /** Returns an iterator currently referencing the start of the container
     */
    StorageContainerIterator<T> * MakeIterator();

    /** Frees the container array and all elements inside it
     */
    ~HashMapStorageContainer()
    {
        for(typename std::map<uint32, T*>::iterator itr = _map.begin(); itr != _map.end(); ++itr)
            delete itr->second;
    }

    /** Do we need to get the max?
     */
    bool NeedsMax()
    {
        return false;
    }

    /** Creates the array with specified maximum
     */
    void Setup(uint32 Max)
    {

    }

    void Resetup(uint32 Max)
    {

    }

    /** Allocates entry Entry in the array and sets the pointer, and returns
     * the allocated memory.
     */
    T * AllocateEntry(uint32 Entry)
    {
        if(_map.find(Entry) != _map.end())
            return reinterpret_cast<T*>(0);
        T *n = RONIN_UTIL::Allocate<T>();
        _map.insert( std::make_pair( Entry, n ) );
        return n;
    }

    /** Deallocates the entry Entry in the array and sets the pointer to null.
     */
    bool DeallocateEntry(uint32 Entry)
    {
        typename std::map<uint32, T*>::iterator itr = _map.find(Entry);
        if(itr == _map.end())
            return false;

        delete itr->second;
        _map.erase(itr);
        return true;
    }


    T * LookupEntry(uint32 Entry)
    {
        typename std::map<uint32, T*>::iterator itr = _map.find(Entry);
        if(itr == _map.end())
            return reinterpret_cast<T*>(0);
        return itr->second;
    }

    /** Sets the pointer to entry Entry to Pointer, and if it already exists frees the existing
     * element.
     */
    bool SetEntry(uint32 Entry, T * Pointer)
    {
        typename std::map<uint32, T*>::iterator itr = _map.find(Entry);
        if(itr == _map.end())
        {
            _map.insert( std::make_pair( Entry, Pointer ) );
            return true;
        }

        delete itr->second;
        itr->second = Pointer;
        return true;
    }

    /** Returns the current pointer if it exists, otherwise allocates it.
     */
    T * LookupEntryAllocate(uint32 Entry)
    {
        if(T * ret = LookupEntry(Entry))
            return ret;
        return AllocateEntry(Entry);
    }

    /** Deletes all entries in the container.
     */
    void Clear()
    {
        typename std::map<uint32, T*>::iterator itr = _map.begin();
        for(; itr != _map.end(); ++itr)
            delete itr->second;
        _map.clear();
    }
};

template<class T>
class SERVER_DECL HashMapStorageIterator : public StorageContainerIterator<T>
{
    HashMapStorageContainer<T> * Source;
    typename std::map<uint32, T*>::iterator itr;
public:

    /** Constructor
    */
    HashMapStorageIterator(HashMapStorageContainer<T> * S) : StorageContainerIterator<T>(), Source(S)
    {
        itr = S->_map.begin();
        if(itr == S->_map.end())
            StorageContainerIterator<T>::Set(0);
        else StorageContainerIterator<T>::Set(itr->second);
    }

    /** Gets the next element, or if we reached the end sets it to 0
    */
    void GetNextElement()
    {
        ++itr;
        if(itr == Source->_map.end())
            StorageContainerIterator<T>::Set(0);
        else StorageContainerIterator<T>::Set(itr->second);
    }

    /** Returns true if we're not at the end, otherwise false.
    */
    bool Inc()
    {
        GetNextElement();
        if(StorageContainerIterator<T>::Pointer != 0)
            return true;
        return false;
    }

    /** Frees the memory occupied by this iterator
    */
    void Destruct()
    {
        delete this;
    }
};

#ifndef SCRIPTLIB
template<class T> StorageContainerIterator<T> * HashMapStorageContainer<T>::MakeIterator() { return new HashMapStorageIterator<T>(this); }
#endif

template<class T, class StorageType> class SERVER_DECL Storage
{
protected:
    StorageType _storage;
    std::string _indexName;
    std::string _formatString;
    std::set<char*> _stringData;
public:

    RONIN_INLINE const char * GetIndexName() { return _indexName.c_str(); }
    RONIN_INLINE const char * GetFormatString() { return _formatString.c_str(); }

    /** False constructor to fool compiler
     */
    Storage() {}
    virtual ~Storage() {}

    /** Makes an iterator, w00t!
     */
    StorageContainerIterator<T> * MakeIterator() { return _storage.MakeIterator(); }

    /** Calls the storage container lookup function.
     */
    T * LookupEntry(uint32 Entry) { return _storage.LookupEntry(Entry); }

    /** Reloads the content in this container.
     */
    virtual void Reload(DirectDatabase *db) = 0;

    /** Loads the container using the specified name and format string
     */
    virtual void Load(DirectDatabase *db, const char * IndexName, const char * FormatString)
    {
        _indexName = IndexName;
        _formatString = FormatString;
    }

    /** Frees the duplicated strings and all entries inside the storage container
     */
    virtual void Cleanup()
    {
        _storage.Clear();
        _indexName.clear();
        _formatString.clear();
        while(!_stringData.empty())
        {
            char *strData = *_stringData.begin();
            _stringData.erase(_stringData.begin());
            free(strData);
        }
        _stringData.clear();
    }
};

template<class T, class StorageType>
class SERVER_DECL SQLStorage : public Storage<T, StorageType>
{
public:
    SQLStorage() : Storage<T, StorageType>() {}
    ~SQLStorage() {}

    /** Creates a new block.
     */
    T* CreateBlock(uint32 entry)
    {
        return Storage<T, StorageType>::_storage.AllocateEntry(entry);
    }

    /** Sets up a new block.
     */
    void SetBlock(uint32 entry, T* p)
    {
        Storage<T, StorageType>::_storage.SetEntry(entry, p);
    }

    /** Loads the block using the format string.
     */
    RONIN_INLINE bool LoadBlock(Field * fields, T * Allocated, bool reload, bool hasIndicator )
    {
        Field *f = fields;
        const char *p = GetFormatString();
        if(hasIndicator)
        {
            for(; *p != 0; ++p, ++f)
                if(*p == 'X' && f->GetBool() == false)
                    return false;
        }

        f = fields;
        uint32 offset = 0;
        p = GetFormatString();
        static const char *null_str = "";
        char *structpointer = (char*)Allocated;
        for(; *p != 0; ++p, ++f)
        {
            switch(*p)
            {
            case 'b':   // Boolean
                {
                    *(bool*)&structpointer[offset] = f->GetBool();
                    offset += sizeof(bool);
                }break;

            case 'c':   // Char
                {
                    *(uint8*)&structpointer[offset] = f->GetUInt8();
                    offset += sizeof(uint8);
                }break;

            case 'h':   // Short
                {
                    *(uint16*)&structpointer[offset] = f->GetUInt16();
                    offset += sizeof(uint16);
                }break;

            case 'u':   // Unsigned integer
                {
                    *(uint32*)&structpointer[offset] = f->GetUInt32();
                    offset += sizeof(uint32);
                }break;

            case 'i':   // Signed integer
                {
                    *(int32*)&structpointer[offset] = f->GetInt32();
                    offset += sizeof(int32);
                }break;

            case 'f':   // Float
                {
                    *(float*)&structpointer[offset] = f->GetFloat();
                    offset += sizeof(float);
                }break;

            case 'S':
                {
                    const char* ptr = f->GetString();
                    ((std::string*)&structpointer[offset])->copy((char*)ptr, strlen(ptr));
                    offset += sizeof(std::string);
                }break;

            case 's':   // Null-terminated string
                {
                    char *ptr = (char*)null_str;
                    if(const char *str = f->GetString())
                        _stringData.insert((ptr = strdup(str)));
                    *(char**)&structpointer[offset] = ptr;
                    offset += sizeof(char*);
                }break;

            case 'X':   // Indicator
            case 'x':   // Skip
                break;

            default:    // unknown
                printf("Unknown field type in string: `%c`\n", *p);
                break;
            }
        }
        return true;
    }

    /** Loads from the table.
     */
    void Load(DirectDatabase *db, std::string IndexName, std::string FormatString)
    {
        Storage<T, StorageType>::Load(db, IndexName.c_str(), FormatString.c_str());
        QueryResult * result;
        if(_storage.NeedsMax())
        {
            uint32 Max = STORAGE_ARRAY_MAX;
            if(result = db->Query("SELECT MAX(entry) FROM %s ORDER BY `entry`", IndexName.c_str()))
            {
                Max = result->Fetch()[0].GetUInt32() + 1;
                if(Max > STORAGE_ARRAY_MAX)
                {
                    sLog.Warning("Storage", "The table, '%s', has been limited to maximum of %u entries. Any entry higher than %u will be discarded.", IndexName.c_str(), STORAGE_ARRAY_MAX, Max );
                    Max = STORAGE_ARRAY_MAX;
                }
                delete result;
            }

            _storage.Setup(Max);
        }

        if(result = db->Query("SELECT * FROM %s", IndexName.c_str()))
        {
            size_t cols = FormatString.length();
            if(result->GetFieldCount() != cols)
            {
                sLog.Error("Storage", "Invalid format in %s (%u/%u), not enough data to proceed.\n", IndexName.c_str(), (unsigned int)result->GetFieldCount(), (unsigned int)cols);
                delete result;
                return;
            }

            bool hasIndicator = FormatString.find("X") != std::string::npos;
            do
            {
                Field * fields = result->Fetch();
                uint32 Entry = fields[0].GetUInt32();
                if(T *Allocated = _storage.AllocateEntry(Entry))
                    if(!LoadBlock(fields, Allocated, false, hasIndicator))
                        _storage.DeallocateEntry(Entry);
            } while(result->NextRow());
            sLog.Notice("Storage", "%u entries loaded from table %s.", result->GetRowCount(), IndexName.c_str());
            delete result;

            //sLog.Success("Storage", "Loaded database cache from `%s`.", IndexName);
        }
    }

    /** Reloads the storage container
     */
    void Reload(DirectDatabase *db)
    {
        sLog.Notice("Storage", "Reloading database cache from `%s`...\n", GetIndexName());
        QueryResult * result = db->Query("SELECT MAX(entry) FROM %s", GetIndexName());
        if(result == 0)
            return;

        uint32 Max = result->Fetch()[0].GetUInt32();
        delete result;
        if(Max == 0)
            return;

        if(_storage.NeedsMax())
        {
            if(Max > STORAGE_ARRAY_MAX-1)
                Max = STORAGE_ARRAY_MAX-1;
            _storage.Resetup(Max+1);
        }

        if(result = db->Query("SELECT * FROM %s", GetIndexName()))
        {
            size_t cols = _formatString.length();
            if(result->GetFieldCount() != cols)
            {
                sLog.Error("Storage", "Invalid format in %s (%u/%u).", GetIndexName(), (unsigned int)cols, (unsigned int)result->GetFieldCount());
                delete result;
                return;
            }

            bool hasIndicator = _formatString.find("X") != std::string::npos;
            do
            {
                Field * fields = result->Fetch();
                uint32 Entry = fields[0].GetUInt32();
                if(T *Allocated = _storage.LookupEntryAllocate(Entry))
                    if(!LoadBlock(fields, Allocated, true, hasIndicator))
                        _storage.DeallocateEntry(Entry);
            } while(result->NextRow());
            delete result;
        }
    }
};
