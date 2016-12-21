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

class UpdateMask
{
public:
    UpdateMask() : mCount(0), mBlocks(0), mUpdateMask(0) { }
    UpdateMask(uint32 count) : mCount(0), mBlocks(0), mUpdateMask(0) { SetCount(count); }
    UpdateMask(UpdateMask const&mask) : mUpdateMask(NULL)
    {
        SetCount(mask.GetCount());
        memcpy(mUpdateMask, mask.mUpdateMask, mask.GetLength());
    }

    ~UpdateMask() { CleanupMask(); }

    RONIN_INLINE bool IsEmpty() { return mUpdateMask == NULL; }
    RONIN_INLINE void CleanupMask() { if(mUpdateMask) delete[] mUpdateMask; mUpdateMask = NULL; }

    RONIN_INLINE void SetBit(uint32 index) { if(mUpdateMask == NULL) return; mUpdateMask[index>>3] |= 1 << (index & 0x7); }
    RONIN_INLINE void UnsetBit(uint32 index) { if(mUpdateMask == NULL) return; mUpdateMask[index>>3] &= ~(1 << (index & 0x7)); }
    RONIN_INLINE bool GetBit(uint32 index) const { if(mUpdateMask == NULL) return false; return (mUpdateMask[index>>3] & (1 << (index & 0x7))) != 0; }

    RONIN_INLINE uint32 GetBlockCount() const { return mBlocks>>2; }
    RONIN_INLINE uint32 GetLength() const { return mBlocks; }
    RONIN_INLINE uint32 GetCount() const { return mCount; }
    RONIN_INLINE uint8* GetMask() { return mUpdateMask; }

    RONIN_INLINE uint8 GetBlock(uint32 index) { if(mUpdateMask == NULL) return 0; return mUpdateMask[index]; }
    RONIN_INLINE void SetBlock(uint32 index, uint8 value) { if(mUpdateMask == NULL) return; mUpdateMask[index] = value; }

    RONIN_INLINE void SetCount(uint32 valuesCount)
    {
        if(mUpdateMask)
            delete[] mUpdateMask;

        mBlocks = (mCount = valuesCount)+7;
        mBlocks >>= 5; mBlocks += 1; mBlocks <<= 2;

        mUpdateMask = new uint8[mBlocks];
        memset(mUpdateMask, 0, mBlocks);
    }

    RONIN_INLINE void Clear()
    {
        if (mUpdateMask)
            memset(mUpdateMask, 0, GetLength());
    }

    UpdateMask& operator = (const UpdateMask& mask)
    {
        SetCount(mask.mCount);
        memcpy(mUpdateMask, mask.mUpdateMask, GetLength());
        return *this;
    }

    void operator &= (const UpdateMask& mask)
    {
        ASSERT(mask.mCount <= mCount);
        for (uint32 i = 0; i < mBlocks; ++i)
            mUpdateMask[i] &= mask.mUpdateMask[i];
    }

    void operator |= (const UpdateMask& mask)
    {
        ASSERT(mask.mCount <= mCount);
        for (uint32 i = 0; i < mBlocks; ++i)
            mUpdateMask[i] |= mask.mUpdateMask[i];
    }

    UpdateMask operator & (const UpdateMask& mask) const
    {
        ASSERT(mask.mCount <= mCount);

        UpdateMask newmask;
        newmask = *this;
        newmask &= mask;
        return newmask;
    }

    UpdateMask operator | (const UpdateMask& mask) const
    {
        ASSERT(mask.mCount <= mCount);

        UpdateMask newmask;
        newmask = *this;
        newmask |= mask;
        return newmask;
    }

private:
    uint32 mCount;
    uint32 mBlocks;
    uint8 *mUpdateMask;
};
