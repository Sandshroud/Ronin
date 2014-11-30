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
    std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);

    time_t convTimePeriod ( uint32 dLength, char dType);
    int32 GetTimePeriodFromString(const char * str);
    std::string ConvertTimeStampToString(uint32 timestamp);
    std::string ConvertTimeStampToDataTime(uint32 timestamp);

    uint32 secsToTimeBitFields(time_t secs);
    void reverse_array(uint8 * pointer, size_t count);
    bool FindXinYString(std::string x, std::string y);
    void TOLOWER(std::string& str);
    void TOUPPER(std::string& str);
    std::string TOLOWER_RETURN(std::string str);
    std::string TOUPPER_RETURN(std::string str);

    // returns true if the ip hits the mask, otherwise false
    bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits);
    uint MakeIP(const char * str);

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
};