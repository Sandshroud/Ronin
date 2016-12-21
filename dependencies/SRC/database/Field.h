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

class SERVER_DECL Field
{
public:

    RONIN_INLINE void SetValue(char* value) { mValue = value; }

    RONIN_INLINE const char *GetString() { return mValue; }
    RONIN_INLINE float GetFloat() { return mValue ? static_cast<float>(atof(mValue)) : 0; }
    RONIN_INLINE bool GetBool() { return mValue ? atoi(mValue) > 0 : false; }
    RONIN_INLINE uint8 GetUInt8() { return mValue ? static_cast<uint8>(atol(mValue)) : 0; }
    RONIN_INLINE int8 GetInt8() { return mValue ? static_cast<int8>(atoi(mValue)) : 0; }
    RONIN_INLINE uint16 GetUInt16() { return mValue ? static_cast<uint16>(atol(mValue)) : 0; }
    RONIN_INLINE int16 GetInt16() { return mValue ? static_cast<int16>(atoi(mValue)) : 0; }
    RONIN_INLINE uint32 GetUInt32() { return mValue ? static_cast<uint32>(atol(mValue)) : 0; }
    RONIN_INLINE int32 GetInt32() { return mValue ? static_cast<int32>(atoi(mValue)) : 0; }
    uint64 GetUInt64() 
    {
        if(mValue)
        {
            uint64 value;
#if defined(_WIN64)
            sscanf(mValue,I64FMTD,(long long unsigned int*)&value);
#else
            sscanf(mValue,I64FMTD,&value);
#endif
            return value;
        }
        else
            return 0;
    }

private:
        char *mValue;
};
