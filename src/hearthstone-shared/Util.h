/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

/* update this every loop to avoid the time() syscall! */
extern SERVER_DECL time_t UNIXTIME;
extern SERVER_DECL tm g_localTime;

///////////////////////////////////////////////////////////////////////////////
// String Functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);

time_t convTimePeriod ( uint32 dLength, char dType);
HEARTHSTONE_INLINE uint32 secsToTimeBitFields(time_t secs)
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
