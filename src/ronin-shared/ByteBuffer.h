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

#include "Common.h"
#include "WoWGuid.h"
#include "LocationVector.h"

class ByteBufferException
{
public:
    ByteBufferException(bool _add, size_t _pos, size_t _esize, size_t _size)
        : add(_add), pos(_pos), esize(_esize), size(_size)
    {
        PrintPosError();
    }

    void PrintPosError() const
    {
        sLog.outError("Attempted to %s in ByteBuffer (pos: %u size: %u) value with size: %u",
            (add ? "put" : "get"), (uint32)pos, (uint32)size, (uint32)esize);
    }
private:
    bool add;
    size_t pos;
    size_t esize;
    size_t size;
};

class SERVER_DECL ByteBuffer
{
public:
    typedef struct{ } FillerType;
    static const FillerType Filler;

    const static size_t DEFAULT_SIZE = 0x1000;

    ByteBuffer(): _rpos(0), _wpos(0), _bitpos(8), _curbitval(0)
    {
        _storage.reserve(DEFAULT_SIZE);
    }

    ByteBuffer(size_t res): _rpos(0), _wpos(0), _bitpos(8), _curbitval(0)
    {
        _storage.reserve(res);
    }

    ByteBuffer(const ByteBuffer &buf): _rpos(buf._rpos), _wpos(buf._wpos), _bitpos(buf._bitpos), _curbitval(buf._curbitval), _storage(buf._storage) { }
    virtual ~ByteBuffer() {}

    void clear()
    {
        _storage.clear();
        _rpos = _wpos = _curbitval = 0;
        _bitpos = 8;
    }

    template <typename T> void append(T value)
    {
        EndianConvert(value);
        append((uint8 *)&value, sizeof(value));
    }

    void FlushBits()
    {
        if (_bitpos == 8)
            return;

        if (_storage.size() < _wpos + 1) _storage.resize(_wpos + 1);
        memcpy(&_storage[_wpos], (uint8 *)&_curbitval, sizeof(uint8));
        _curbitval = 0;
        _bitpos = 8;
        _wpos += 1;
    }

    bool WriteBit(uint32 bit)
    {
        --_bitpos;
        if (bit) _curbitval |= (1 << (_bitpos));
        if (_bitpos == 0)
            FlushBits();
        return (bit != 0);
    }

    void WriteBitString(uint32 count, ...)
    {
        va_list vl;
        va_start(vl, count);
        for(uint32 i = 0; i < count; i++)
            WriteBit(va_arg(vl, uint32));
        va_end(vl);
    }

    void WriteGuidBitString(uint32 count, WoWGuid guid, FillerType filler, ...)
    {
        va_list vl;
        va_start(vl, filler);
        for(uint32 i = 0; i < count; i++)
            WriteBit(guid[va_arg(vl, uint32)]);
        va_end(vl);
    }

    bool ReadBit()
    {
        ++_bitpos;
        if (_bitpos > 7)
        {
            _bitpos = 0;
            _curbitval = read<uint8>();
        }

        return ((_curbitval >> (7-_bitpos)) & 1) != 0;
    }

    template <typename T> void WriteBits(T value, size_t bits)
    {
        for (int32 i = bits-1; i >= 0; --i)
            WriteBit((value >> i) & 1);
    }

    uint32 ReadBits(size_t bits)
    {
        uint32 value = 0;
        for (int32 i = bits-1; i >= 0; --i)
            if (ReadBit()) value |= (1 << (i));
        return value;
    }

    // Read guid mask bits
    void ReadGuidBitString(uint32 count, WoWGuid &guid, FillerType filler, ...)
    {
        va_list vl;
        va_start(vl, filler);
        for(uint32 i = 0; i < count; i++)
            guid[va_arg(vl, uint32)] = ReadBit();
        va_end(vl);
    }

    void ReadGuidByteString(uint32 count, WoWGuid &guid, FillerType filler, ...)
    {
        va_list vl;
        va_start(vl, filler);
        for(uint32 i = 0; i < count; i++)
            ReadByteSeq(guid[(va_arg(vl, uint32))]);
        va_end(vl);
    }

    // Reads a byte (if needed) in-place
    void ReadByteSeq(uint8& b)
    {
        if (b != 0)
            b ^= read<uint8>();
    }

    void WriteByteSeq(uint8 b)
    {
        if (b != 0)
            append<uint8>(b ^ 1);
    }

    void WriteSeqByteString(int guid, uint32 count, ...) = delete;
    void WriteSeqByteString(uint32 count, WoWGuid guid, FillerType filler, ...)
    {
        va_list vl;
        va_start(vl, filler);
        for(uint32 i = 0; i < count; i++)
            WriteByteSeq(guid[(va_arg(vl, uint32))]);
        va_end(vl);
    }

    template <typename T> void put(size_t pos,T value)
    {
        EndianConvert(value);
        put(pos,(uint8 *)&value,sizeof(value));
    }

    // stream like operators for storing data
    ByteBuffer &operator<<(bool value)
    {
        append<char>((char)value);
        return *this;
    }

    // unsigned
    ByteBuffer &operator<<(uint8 value)
    {
        append<uint8>(value);
        return *this;
    }
    ByteBuffer &operator<<(uint16 value)
    {
        append<uint16>(value);
        return *this;
    }

    ByteBuffer &operator<<(uint32 value)
    {
        append<uint32>(value);
        return *this;
    }

    ByteBuffer &operator<<(uint64 value)
    {
        append<uint64>(value);
        return *this;
    }

    // signed as in 2e complement
    ByteBuffer &operator<<(int8 value)
    {
        append<int8>(value);
        return *this;
    }

    ByteBuffer &operator<<(int16 value)
    {
        append<int16>(value);
        return *this;
    }

    ByteBuffer &operator<<(int32 value)
    {
        append<int32>(value);
        return *this;
    }

    ByteBuffer &operator<<(int64 value)
    {
        append<int64>(value);
        return *this;
    }

    ByteBuffer &operator<<(float value)
    {
        append<float>(value);
        return *this;
    }

    ByteBuffer &operator<<(double value)
    {
        append<double>(value);
        return *this;
    }

    ByteBuffer &operator<<(const std::string &value)
    {
        append((uint8 *)value.c_str(), value.length());
        append((uint8)0);
        return *this;
    }

    ByteBuffer &operator<<(const char *str)
    {
        append((uint8 *)str, strlen(str));
        append((uint8)0);
        return *this;
    }

    ByteBuffer &operator<<(WoWGuid value)
    {
        append<uint64>(value);
        return *this;
    }

    ByteBuffer &operator<<(WGuidPacked *value)
    {
        uint64 guid = value->m_guid->raw();
        uint8 len = 1, packGUID[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        for (uint8 i = 0; guid != 0; ++i)
        {
            if (guid & 0xFF)
            {
                packGUID[0] |= uint8(1 << i);
                packGUID[len] = uint8(guid & 0xFF);
                ++len;
            }

            guid >>= 8;
        }
        append(packGUID, len);
        delete value;
        return *this;
    }

    // stream like operators for reading data
    ByteBuffer &operator>>(bool &value)
    {
        value = read<char>() > 0 ? true : false;
        return *this;
    }

    //unsigned
    ByteBuffer &operator>>(uint8 &value)
    {
        value = read<uint8>();
        return *this;
    }

    ByteBuffer &operator>>(uint16 &value)
    {
        value = read<uint16>();
        return *this;
    }

    ByteBuffer &operator>>(uint32 &value)
    {
        value = read<uint32>();
        return *this;
    }

    ByteBuffer &operator>>(uint64 &value)
    {
        value = read<uint64>();
        return *this;
    }

    //signed as in 2e complement
    ByteBuffer &operator>>(int8 &value)
    {
        value = read<int8>();
        return *this;
    }

    ByteBuffer &operator>>(int16 &value)
    {
        value = read<int16>();
        return *this;
    }

    ByteBuffer &operator>>(int32 &value)
    {
        value = read<int32>();
        return *this;
    }

    ByteBuffer &operator>>(int64 &value)
    {
        value = read<int64>();
        return *this;
    }

    ByteBuffer &operator>>(float &value)
    {
        value = read<float>();
        return *this;
    }

    ByteBuffer &operator>>(double &value)
    {
        value = read<double>();
        return *this;
    }

    ByteBuffer &operator>>(WoWGuid &value)
    {
        value = read<uint64>();
        return *this;
    }

    ByteBuffer &operator>>(WGuidPacked *value)
    {
        WoWGuid *guid = value->m_guid;
        uint8 guidmark = read<uint8>();
        for (uint8 i = 0; i < 8; ++i)
        {
            uint8 byte = 0;
            if (guidmark & (uint8(0x01) << i))
                byte = read<uint8>();
            guid->Set(i, byte);
        }

        return *this;
    }

    ByteBuffer &operator>>(std::string& value)
    {
        value.clear();
        while(true)
        {
            char c = read<char>();
            if (c == 0)
                break;
            value += c;
        }
        return *this;
    }

    uint8 operator[](size_t pos)
    {
        return read<uint8>(pos);
    }

    size_t rpos()
    {
        return _rpos;
    }

    size_t rpos(size_t rpos)
    {
        _rpos = rpos;
        return _rpos;
    }

    void rfinish()
    {
        _rpos = wpos();
    }

    size_t wpos()
    {
        return _wpos;
    }

    size_t wpos(size_t wpos)
    {
        _wpos = wpos;
        return _wpos;
    }

    size_t bitpos()
    {
        return 8-_bitpos;
    }

    size_t bitpos(size_t bitpos)
    {
        _bitpos = bitpos;
        return _bitpos;
    }

    template<typename T> void read_skip() { read_skip(sizeof(T)); }
    template<> inline void read_skip<char*>() { std::string temp; *this >> temp; }
    template<> inline void read_skip<char const*>() { read_skip<char*>(); }
    template<> inline void read_skip<std::string>() { read_skip<char*>(); }

    void read_skip(size_t skip)
    {
        if(_rpos + skip > size())
            throw ByteBufferException(false, _rpos, skip, size());
        _rpos += skip;
    }

    template <typename T> T read()
    {
        if(_rpos + sizeof(T) > size())
            throw ByteBufferException(false, _rpos, sizeof(T), size());
        T r = read<T>(_rpos);
        _rpos += sizeof(T);
        return r;
    }

    template <typename T> T read(size_t pos) const
    {
        if(pos + sizeof(T) > size())
            throw ByteBufferException(false, pos, sizeof(T), size());
        T val = *((T const*)&_storage[pos]);
        EndianConvert(&val);
        return val;
    }

    void read(uint8 *dest, size_t len)
    {
        if(_rpos  + len > size())
            throw ByteBufferException(false, _rpos, len, size());
        memcpy(dest, &_storage[_rpos], len);
        _rpos += len;
    }

    void readvector(LocationVector &v, bool orientation = false)
    {
        v.x = read<float>();
        v.y = read<float>();
        v.z = read<float>();
        if(orientation)
            v.o = read<float>();
    }

    const uint8 *contents(size_t pos = 0) const { if(size() <= pos) return NULL; return &_storage[pos]; };

    RONIN_INLINE size_t size() const { return _storage.size(); };

    // one should never use resize probably
    void resize(size_t newsize)
    {
        _storage.resize(newsize);
        _rpos = 0;
        _wpos = size();
    }

    void reserve(size_t ressize)
    {
        if (ressize > size()) _storage.reserve(ressize);
    }

    // append bytes of value 0
    void zerofill(size_t len) { for(size_t i = 0; i < len; i++) append<uint8>(0); }

    // appending to the end of buffer
    void append(const std::string& str) { append((uint8 *)str.c_str(), str.size() + 1); }
    template<class T> void append(const T *src, size_t cnt) { return append((const uint8 *)src, cnt * sizeof(T)); }
    void append(const uint8 *src, size_t cnt)
    {
        if(cnt == 0 || src == 0)
            return;

        FlushBits();
        if (_storage.size() < _wpos + cnt)
            _storage.resize(_wpos + cnt);

        memcpy(&_storage[_wpos], src, cnt);
        _wpos += cnt;
    }

    void append(ByteBuffer buffer) = delete; // Do not append byte buffers
    void append(ByteBuffer *buffer) = delete; // Do not append byte buffers
    void appendvector(const LocationVector &v, bool orientation = false)
    {
        append<float>(v.x);
        append<float>(v.y);
        append<float>(v.z);
        if(orientation)
            append<float>(v.o);
    }

    void put(size_t pos, const uint8 *src, size_t cnt)
    {
        if(pos + cnt > size())
            throw ByteBufferException(true, pos, cnt, size());
        memcpy(&_storage[pos], src, cnt);
    }
    //void insert(size_t pos, const uint8 *src, size_t cnt) {
    //  std::copy(src, src + cnt, inserter(_storage, _storage.begin() + pos));
    //}

    // Reads a string without a null terminator
    std::string ReadString(uint32 length)
    {
        std::string retval;
        if(length)
        {
            char* buffer = new char[length + 1]();
            read((uint8*)buffer, length);
            retval.append(buffer);
            delete[] buffer;
        }
        return retval;
    }

    // Does not write null terminator
    void WriteString(std::string const& str)
    {
        if (size_t len = str.length())
            append(str.c_str(), len);
    }

    void hexlike()
    {
        if(sLog.GetLogLevel() >= 5)
        {
            uint32 j = 1, k = 1;
            printf("STORAGE_SIZE: %u\n", (unsigned int)size() );
            for(uint32 i = 0; i < size(); i++)
            {
                if ((i == (j*8)) && ((i != (k*16))))
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        printf("| 0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        printf("| %X ", read<uint8>(i) );
                    }

                    j++;
                }
                else if (i == (k*16))
                {
                    rpos(rpos()-16);    // move read pointer 16 places back
                    printf(" | ");    // write split char

                    for (int x = 0; x < 16; x++)
                    {
                        printf("%c", read<uint8>(i-16 + x) );
                    }

                    if (read<uint8>(i) < 0x0F)
                    {
                        printf("\n0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        printf("\n%X ", read<uint8>(i) );
                    }

                    k++;
                    j++;
                }
                else
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        printf("0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        printf("%X ", read<uint8>(i) );
                    }
                }
            }
            printf("\n");
        }
    }

    void hexlike(FILE *output)
    {
        uint32 j = 1, k = 1;
        fprintf(output, "STORAGE_SIZE: %u\r\n", (unsigned int)size() );
        for(uint32 i = 0; i < size(); i++)
        {
            if ((i == (j*8)) && ((i != (k*16))))
            {
                if (read<uint8>(i) <= 0x0F)
                {
                    fprintf(output, "| 0%X ", read<uint8>(i) );
                }
                else
                {
                    fprintf(output, "| %X ", read<uint8>(i) );
                }

                j++;
            }
            else if (i == (k*16))
            {
                rpos(rpos()-16);    // move read pointer 16 places back
                fprintf(output, " | ");    // write split char

                for (int x = 0; x < 16; x++)
                {
                    uint8 val = read<uint8>(i-16 + x);
                    fprintf(output, "%c", isprint(val) ? val : '.');
                }

                if (read<uint8>(i) <= 0x0F)
                {
                    fprintf(output, "\r\n0%X ", read<uint8>(i) );
                }
                else
                {
                    fprintf(output, "\r\n%X ", read<uint8>(i) );
                }

                k++;
                j++;
            }
            else
            {
                if (read<uint8>(i) <= 0x0F)
                {
                    fprintf(output, "0%X ", read<uint8>(i) );
                }
                else
                {
                    fprintf(output, "%X ", read<uint8>(i) );
                }
            }
        }
        fprintf(output, "\r\n\r\n");
    }

    void bithexlike(FILE *output)
    {
        uint32 j = 1, k = 1;
        fprintf(output, "STORAGE_SIZE: %u\r\n", (unsigned int)size() );
        for(uint32 i = 0; i < size(); i++)
        {
            uint8 val = read<uint8>(i);
            for(uint8 x = 0; x < 8; x++)
                fprintf(output, "%u", ((val&1<<x) ? 1 : 0));
            if(i > 0 && i%8 == 0)
                fprintf(output, "\n");
            else fprintf(output, " ");
        }
        if(_bitpos != 8)
        {
            for(uint8 x = 0; x < 8; x++)
                fprintf(output, "%u", ((_curbitval&x<<1) ? 1 : 0));
        }
        fprintf(output, "\r\n\r\n");
    }

    RONIN_INLINE void reverse()
    {
        std::reverse(_storage.begin(), _storage.end());
    }

protected:
    uint8 _curbitval; // bit operation val
    // read and write positions
    size_t _rpos, _wpos, _bitpos;
    std::vector<uint8> _storage;
};

class SERVER_DECL BitBuffer
{
public:
    void SetBit(bool bit) { _storage.push_back(bit); };
    template <typename T> void SetBits(T value, size_t bits)
    {
        for (int32 i = bits-1; i >= 0; --i)
            SetBit((value >> i) & 1);
    }

    void Append(ByteBuffer *buff, bool flushBits)
    {
        while(!_storage.empty())
        {
            bool res = _storage.front();
            _storage.pop_front();
            buff->WriteBit(res);
        }

        if(!flushBits)
            return;
        buff->FlushBits();
    }

private:
    std::deque<bool> _storage;
};

RONIN_INLINE void FastGUIDPack(ByteBuffer & buf, const uint64 & oldguid)
{
    uint8 guidmask = 0,  fieldcount = 1;
    uint8 guidfields[9] = {0,0,0,0,0,0,0,0};
    for(uint8 x = 0; x < 8; ++x)
    {
        uint8 p = ((uint8*)&oldguid)[x];
        if(p)
        {
            guidfields[fieldcount++] = p;
            guidmask |= 1 << x;
        }
    }
    guidfields[0] = guidmask;
    buf.append(guidfields, fieldcount);
}

RONIN_INLINE unsigned int FastGUIDPack(const uint64 & oldguid, unsigned char * buffer, uint32 pos)
{
    uint8 guidmask = 0;
    uint8 guidfields[9] = {0,0,0,0,0,0,0,0};
    for(uint8 x = 0, i = 0; x < 8; ++x)
    {
        uint8 p = ((uint8*)&oldguid)[x];
        if(p)
        {
            guidfields[i++] = p;
            guidmask |= 1 << x;
        }
    }
    uint32 buffPos = pos;
    buffer[buffPos++] = guidmask;
    for(uint8 x = 0; x < 8; x++)
        if(guidfields[x])
            buffer[buffPos++] = guidfields[x];
    return (buffPos - pos);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
    b << (uint32)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++) {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--) {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
    b << (uint32)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); i++) {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--) {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
    b << (uint32)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); i++) {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
    uint32 msize;
    b >> msize;
    m.clear();
    while(msize--) {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}
