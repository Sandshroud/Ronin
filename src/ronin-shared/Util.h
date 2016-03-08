/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

/* update this every loop to avoid the time() syscall! */
extern SERVER_DECL time_t UNIXTIME;
extern SERVER_DECL tm g_localTime;

namespace RONIN_UTIL
{
    static const char* timeNames[6] = { " seconds, ", " minutes, ", " hours, ", " days, ", " months, ", " years, " };
    static const char * szDayNames[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    static const char * szMonthNames[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

    ///////////////////////////////////////////////////////////////////////////////
    // String Functions ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    RONIN_INLINE std::vector<std::string> StrSplit(const std::string &src, const std::string &sep)
    {
        std::string s;
        std::vector<std::string> r;
        for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
        {
            if (sep.find(*i) != std::string::npos)
            {
                if (s.length())
                    r.push_back(s);
                s = "";
            } else s += *i;
        }
        if (s.length())
            r.push_back(s);
        return r;
    }

    RONIN_INLINE time_t convTimePeriod ( uint32 dLength, char dType)
    {
        time_t rawtime = 0;
        if (dLength == 0)
            return rawtime;

        struct tm *ti = localtime( &rawtime );
        switch(dType)
        {
        case 'h': ti->tm_hour += dLength; break;
        case 'd': ti->tm_mday += dLength; break;
        case 'w': ti->tm_mday += 7 * dLength; break;
        case 'm': ti->tm_mon += dLength; break;
        case 'y': ti->tm_year += dLength; break;
        default: ti->tm_min += dLength; break;
        }
        return mktime(ti);
    }

    RONIN_INLINE int32 GetTimePeriodFromString(const char * str)
    {
        char *p = (char*)str;
        std::string number_temp; number_temp.reserve(10);
        uint32 time_to_ban = 0, multiplier, multipliee;
        while(*p != 0)
        {
            // always starts with a number.
            if(!isdigit(*p))
                break;

            number_temp.clear();
            while(isdigit(*p) && *p != 0)
            {
                number_temp += *p;
                ++p;
            }

            // try and find a letter
            if(*p == 0)
                break;

            // check the type
            switch(tolower(*p))
            {
            case 'y': multiplier = TIME_YEAR; break;
            case 'm': multiplier = TIME_MONTH; break;
            case 'd': multiplier = TIME_DAY; break;
            case 'h': multiplier = TIME_HOUR; break;
            default: return -1; break;
            }

            ++p;
            multipliee = atoi(number_temp.c_str());
            time_to_ban += (multiplier * multipliee);
        }

        return time_to_ban;
    }

    RONIN_INLINE std::string ConvertTimeStampToString(uint32 timestamp)
    {
        static int calcVal[5] = { 60, 60, 24, 30, 12};
        int timeVals[6] = { timestamp, 0, 0, 0, 0, 0 };
        for(uint8 i = 0; i < 5; i++)
        {
            if(timeVals[i+1] = timeVals[i]/calcVal[i])
                timeVals[i] -= timeVals[i+1]*calcVal[i];
            else break;
        }

        char szTempBuf[100];
        std::string szResult;
        for(int i = 5; i >= 0; i--)
        {
            sprintf(szTempBuf,"%02u", timeVals[i]);
            szResult += szTempBuf;
            szResult += timeNames[i];
        }
        return szResult;
    }

    RONIN_INLINE std::string ConvertTimeStampToDataTime(uint32 timestamp)
    {
        char szTempBuf[100];
        time_t t = (time_t)timestamp;
        struct tm *pTM = localtime(&t);

        std::string szResult;
        szResult += szDayNames[pTM->tm_wday];
        szResult += ", ";
        sprintf(szTempBuf,"%02u", pTM->tm_mday);
        szResult += szTempBuf;
        szResult += " ";
        szResult += szMonthNames[pTM->tm_mon];
        szResult += " ";
        sprintf(szTempBuf,"%u",pTM->tm_year+1900);
        szResult += szTempBuf;
        szResult += ", ";
        sprintf(szTempBuf,"%02u",pTM->tm_hour);
        szResult += szTempBuf;
        szResult += ":";
        sprintf(szTempBuf,"%02u",pTM->tm_min);
        szResult += szTempBuf;
        szResult += ":";
        sprintf(szTempBuf,"%02u",pTM->tm_sec);
        szResult += szTempBuf;
        return szResult;
    }

    RONIN_INLINE uint32 secsToTimeBitFields(time_t secs)
    {
        tm* time = localtime(&secs);
        uint32 Time = ((time->tm_min << 0) & 0x0000003F); // Minute
        Time |= ((time->tm_hour << 6) & 0x000007C0); // Hour
        Time |= ((time->tm_wday << 11) & 0x00003800); // WeekDay
        Time |= (((time->tm_mday-1) << 14) & 0x000FC000); // MonthDay
        Time |= ((time->tm_mon << 20) & 0x00F00000); // Month
        Time |= (((time->tm_year-100) << 24) & 0x1F000000); // Year
        return Time;
    }

    RONIN_INLINE void reverse_array(uint8 * pointer, size_t count)
    {
        size_t x;
        uint8 * temp = (uint8*)malloc(count);
        memcpy(temp, pointer, count);
        for(x = 0; x < count; ++x)
            pointer[x] = temp[count-x-1];
        free(temp);
    }

    RONIN_INLINE bool FindXinYString(std::string x, std::string y)
    {
        return y.find(x) != std::string::npos;
    }

    RONIN_INLINE void TOLOWER(std::string& str)
    {
        for(size_t i = 0; i < str.length(); ++i)
            str[i] = (char)tolower(str[i]);
    }

    RONIN_INLINE void TOUPPER(std::string& str)
    {
        for(size_t i = 0; i < str.length(); ++i)
            str[i] = (char)toupper(str[i]);
    }

    RONIN_INLINE std::string TOLOWER_RETURN(std::string str)
    {
        std::string newname = str;
        for(size_t i = 0; i < str.length(); ++i)
            newname[i] = (char)tolower(str[i]);

        return newname;
    }

    RONIN_INLINE std::string TOUPPER_RETURN(std::string str)
    {
        std::string newname = str;
        for(size_t i = 0; i < str.length(); ++i)
            newname[i] = (char)toupper(str[i]);
        return newname;
    }

    // returns true if the ip hits the mask, otherwise false
    RONIN_INLINE bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits)
    {
        // CIDR bans are a compacted form of IP / Submask
        // So 192.168.1.0/255.255.255.0 would be 192.168.1.0/24
        // IP's in the 192.168l.1.x range would be hit, others not.
        unsigned char * source_ip = (unsigned char*)&IP;
        unsigned char * mask = (unsigned char*)&Mask;
        int full_bytes = MaskBits / 8;
        int leftover_bits = MaskBits % 8;
        //int byte;

        // sanity checks for the data first
        if( MaskBits > 32 )
            return false;

        // this is the table for comparing leftover bits
        static const unsigned char leftover_bits_compare[9] = {
            0x00,           // 00000000
            0x80,           // 10000000
            0xC0,           // 11000000
            0xE0,           // 11100000
            0xF0,           // 11110000
            0xF8,           // 11111000
            0xFC,           // 11111100
            0xFE,           // 11111110
            0xFF,           // 11111111 - This one isn't used
        };

        // if we have any full bytes, compare them with memcpy
        if( full_bytes > 0 )
        {
            if( memcmp( source_ip, mask, full_bytes ) != 0 )
                return false;
        }

        // compare the left over bits
        if( leftover_bits > 0 )
        {
            if( ( source_ip[full_bytes] & leftover_bits_compare[leftover_bits] ) !=
                ( mask[full_bytes] & leftover_bits_compare[leftover_bits] ) )
            {
                // one of the bits does not match
                return false;
            }
        }

        // all of the bits match that were testable
        return true;
    }

    RONIN_INLINE uint MakeIP(const char * str)
    {
        unsigned int bytes[4];
        unsigned int res;
        if( sscanf(str, "%u.%u.%u.%u", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4 )
            return 0;

        res = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        return res;
    }

    template<typename T> RONIN_INLINE T FirstBitValue(T value)
    {
        assert(sizeof(T)<=8); // Limit to 8 bytes
        if(value)
        {   // for each byte we have 8 bit stacks
            for(T i = 0; i < sizeof(T)*8; i++)
                if(value & (T(1)<<i))
                    return i;
        } return static_cast<T>(NULL);
    }

    RONIN_INLINE float PercentFloatVar(float val)
    {
        if (val <= -100.f) val = -99.99f;
        if(val >= 100.f) val = 99.99f;
        return 100.f-val;
    }

    // Determines if either two values are equal, or the difference is less than one hundred thousandth units of value a
    RONIN_INLINE bool fuzzyEq(double a, double b)
    {
        if(a != b)
        {
            const double diff = abs(a-b), aa = abs(a) + 1.0;
            if (aa == std::numeric_limits<double>::infinity())
                return diff < 0.00001f;
            return diff < (0.00001f * aa);
        }
        return true;
    }

    RONIN_INLINE unsigned int getRBitOffset(uint32 value)
    {
        unsigned int ret = 0;
#ifdef WIN32
        _BitScanReverse((LPDWORD)&ret, value);
#else
        ret = __builtin_clz(value);
#endif
        return ret;
    }
};


////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

// $Id$


#include <algorithm>
#include <functional>
#include <vector>
#include <utility>
#include <iterator>
#include <map>


namespace Loki
{
////////////////////////////////////////////////////////////////////////////////
// class template AssocVectorCompare
// Used by AssocVector
////////////////////////////////////////////////////////////////////////////////

    namespace Private
    {
        template <class Value, class C>
        class AssocVectorCompare : public C
        {
            typedef std::pair<typename C::first_argument_type, Value>
                Data;
            typedef typename C::first_argument_type first_argument_type;

        public:
            AssocVectorCompare()
            {}

            AssocVectorCompare(const C& src) : C(src)
            {}

            bool operator()(const first_argument_type& lhs,
                const first_argument_type& rhs) const
            { return C::operator()(lhs, rhs); }

            bool operator()(const Data& lhs, const Data& rhs) const
            { return operator()(lhs.first, rhs.first); }

            bool operator()(const Data& lhs,
                const first_argument_type& rhs) const
            { return operator()(lhs.first, rhs); }

            bool operator()(const first_argument_type& lhs,
                const Data& rhs) const
            { return operator()(lhs, rhs.first); }
        };
    }

////////////////////////////////////////////////////////////////////////////////
// class template AssocVector
// An associative vector built as a syntactic drop-in replacement for std::map
// BEWARE: AssocVector doesn't respect all map's guarantees, the most important
//     being:
// * iterators are invalidated by insert and erase operations
// * the complexity of insert/erase is O(N) not O(log N)
// * value_type is std::pair<K, V> not std::pair<const K, V>
// * iterators are random
////////////////////////////////////////////////////////////////////////////////


    template
    <
        class K,
        class V,
        class C = std::less<K>,
        class A = std::allocator< std::pair<K, V> >
    >
    class AssocVector
        : private std::vector< std::pair<K, V>, A >
        , private Private::AssocVectorCompare<V, C>
    {
        typedef std::vector<std::pair<K, V>, A> Base;
        typedef Private::AssocVectorCompare<V, C> MyCompare;

    public:
        typedef K key_type;
        typedef V mapped_type;
        typedef typename Base::value_type value_type;

        typedef C key_compare;
        typedef A allocator_type;
        typedef typename A::reference reference;
        typedef typename A::const_reference const_reference;
        typedef typename Base::iterator iterator;
        typedef typename Base::const_iterator const_iterator;
        typedef typename Base::size_type size_type;
        typedef typename Base::difference_type difference_type;
        typedef typename A::pointer pointer;
        typedef typename A::const_pointer const_pointer;
        typedef typename Base::reverse_iterator reverse_iterator;
        typedef typename Base::const_reverse_iterator const_reverse_iterator;

        class value_compare
            : public std::binary_function<value_type, value_type, bool>
            , private key_compare
        {
            friend class AssocVector;

        protected:
            value_compare(key_compare pred) : key_compare(pred)
            {}

        public:
            bool operator()(const value_type& lhs, const value_type& rhs) const
            { return key_compare::operator()(lhs.first, rhs.first); }
        };

        // 23.3.1.1 construct/copy/destroy

        explicit AssocVector(const key_compare& comp = key_compare(),
            const A& alloc = A())
        : Base(alloc), MyCompare(comp)
        {}

        template <class InputIterator>
        AssocVector(InputIterator first, InputIterator last,
            const key_compare& comp = key_compare(),
            const A& alloc = A())
        : Base( alloc ), MyCompare( comp )
        {
            typedef ::std::vector< ::std::pair< K, V >, A > BaseType;
            typedef ::std::map< K, V, C, A > TempMap;
            typedef ::std::back_insert_iterator< Base > MyInserter;
            MyCompare & me = *this;
            const A tempAlloc;
            // Make a temporary map similar to this type to prevent any duplicate elements.
            TempMap temp( first, last, me, tempAlloc );
            Base::reserve( temp.size() );
            BaseType & target = static_cast< BaseType & >( *this );
            MyInserter myInserter = ::std::back_inserter( target );
            ::std::copy( temp.begin(), temp.end(), myInserter );
        }

        AssocVector& operator=(const AssocVector& rhs)
        {
            AssocVector(rhs).swap(*this);
            return *this;
        }

        // iterators:
        // The following are here because MWCW gets 'using' wrong
        iterator begin() { return Base::begin(); }
        const_iterator begin() const { return Base::begin(); }
        iterator end() { return Base::end(); }
        const_iterator end() const { return Base::end(); }
        reverse_iterator rbegin() { return Base::rbegin(); }
        const_reverse_iterator rbegin() const { return Base::rbegin(); }
        reverse_iterator rend() { return Base::rend(); }
        const_reverse_iterator rend() const { return Base::rend(); }

        // capacity:
        bool empty() const { return Base::empty(); }
        size_type size() const { return Base::size(); }
        size_type max_size() { return Base::max_size(); }

        // 23.3.1.2 element access:
        mapped_type& operator[](const key_type& key)
        { return insert(value_type(key, mapped_type())).first->second; }

        // modifiers:
        std::pair<iterator, bool> insert(const value_type& val)
        {
            bool found(true);
            iterator i(lower_bound(val.first));

            if (i == end() || this->operator()(val.first, i->first))
            {
                i = Base::insert(i, val);
                found = false;
            }
            return std::make_pair(i, !found);
        }
        //Section [23.1.2], Table 69
        //http://developer.apple.com/documentation/DeveloperTools/gcc-3.3/libstdc++/23_containers/howto.html#4
        iterator insert(iterator pos, const value_type& val)
        {
            if( (pos == begin() || this->operator()(*(pos-1),val)) &&
                (pos == end()    || this->operator()(val, *pos)) )
            {
                return Base::insert(pos, val);
            }
            return insert(val).first;
        }

        template <class InputIterator>
        void insert(InputIterator first, InputIterator last)
        { for (; first != last; ++first) insert(*first); }

        void erase(iterator pos)
        { Base::erase(pos); }

        size_type erase(const key_type& k)
        {
            iterator i(find(k));
            if (i == end()) return 0;
            erase(i);
            return 1;
        }

        void erase(iterator first, iterator last)
        { Base::erase(first, last); }

        void swap(AssocVector& other)
        {
            Base::swap(other);
            MyCompare& me = *this;
            MyCompare& rhs = other;
            std::swap(me, rhs);
        }

        void clear()
        { Base::clear(); }

        // observers:
        key_compare key_comp() const
        { return *this; }

        value_compare value_comp() const
        {
            const key_compare& comp = *this;
            return value_compare(comp);
        }

        // 23.3.1.3 map operations:
        iterator find(const key_type& k)
        {
            iterator i(lower_bound(k));
            if (i != end() && this->operator()(k, i->first))
            {
                i = end();
            }
            return i;
        }

        const_iterator find(const key_type& k) const
        {
            const_iterator i(lower_bound(k));
            if (i != end() && this->operator()(k, i->first))
            {
                i = end();
            }
            return i;
        }

        size_type count(const key_type& k) const
        { return find(k) != end(); }

        iterator lower_bound(const key_type& k)
        {
            MyCompare& me = *this;
            return std::lower_bound(begin(), end(), k, me);
        }

        const_iterator lower_bound(const key_type& k) const
        {
            const MyCompare& me = *this;
            return std::lower_bound(begin(), end(), k, me);
        }

        iterator upper_bound(const key_type& k)
        {
            MyCompare& me = *this;
            return std::upper_bound(begin(), end(), k, me);
        }

        const_iterator upper_bound(const key_type& k) const
        {
            const MyCompare& me = *this;
            return std::upper_bound(begin(), end(), k, me);
        }

        std::pair<iterator, iterator> equal_range(const key_type& k)
        {
            MyCompare& me = *this;
            return std::equal_range(begin(), end(), k, me);
        }

        std::pair<const_iterator, const_iterator> equal_range(
            const key_type& k) const
        {
            const MyCompare& me = *this;
            return std::equal_range(begin(), end(), k, me);
        }

        template <class K1, class V1, class C1, class A1>
        friend bool operator==(const AssocVector<K1, V1, C1, A1>& lhs,
                        const AssocVector<K1, V1, C1, A1>& rhs);

        bool operator<(const AssocVector& rhs) const
        {
            const Base& me = *this;
            const Base& yo = rhs;
            return me < yo;
        }

        template <class K1, class V1, class C1, class A1>
        friend bool operator!=(const AssocVector<K1, V1, C1, A1>& lhs,
                               const AssocVector<K1, V1, C1, A1>& rhs);

        template <class K1, class V1, class C1, class A1>
        friend bool operator>(const AssocVector<K1, V1, C1, A1>& lhs,
                              const AssocVector<K1, V1, C1, A1>& rhs);

        template <class K1, class V1, class C1, class A1>
        friend bool operator>=(const AssocVector<K1, V1, C1, A1>& lhs,
                               const AssocVector<K1, V1, C1, A1>& rhs);

        template <class K1, class V1, class C1, class A1>
        friend bool operator<=(const AssocVector<K1, V1, C1, A1>& lhs,
                               const AssocVector<K1, V1, C1, A1>& rhs);
    };

    template <class K, class V, class C, class A>
    inline bool operator==(const AssocVector<K, V, C, A>& lhs,
                           const AssocVector<K, V, C, A>& rhs)
    {
      const std::vector<std::pair<K, V>, A>& me = lhs;
      return me == rhs;
    }

    template <class K, class V, class C, class A>
    inline bool operator!=(const AssocVector<K, V, C, A>& lhs,
                           const AssocVector<K, V, C, A>& rhs)
    { return !(lhs == rhs); }

    template <class K, class V, class C, class A>
    inline bool operator>(const AssocVector<K, V, C, A>& lhs,
                          const AssocVector<K, V, C, A>& rhs)
    { return rhs < lhs; }

    template <class K, class V, class C, class A>
    inline bool operator>=(const AssocVector<K, V, C, A>& lhs,
                           const AssocVector<K, V, C, A>& rhs)
    { return !(lhs < rhs); }

    template <class K, class V, class C, class A>
    inline bool operator<=(const AssocVector<K, V, C, A>& lhs,
                           const AssocVector<K, V, C, A>& rhs)
    { return !(rhs < lhs); }


    // specialized algorithms:
    template <class K, class V, class C, class A>
    void swap(AssocVector<K, V, C, A>& lhs, AssocVector<K, V, C, A>& rhs)
    { lhs.swap(rhs); }

} // namespace Loki
