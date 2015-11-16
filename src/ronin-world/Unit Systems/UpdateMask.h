/***
 * Demonstrike Core
 */

#pragma once

class UpdateMask
{
public:
    UpdateMask() : mCount(0), mBlocks(0), mUpdateMask(0) { }
    UpdateMask(uint32 count) : mCount(0), mBlocks(0), mUpdateMask(0) { SetCount(count); }
    UpdateMask(const UpdateMask& mask) : mUpdateMask(0) { *this = mask; }

    ~UpdateMask()
    {
        delete[] mUpdateMask;
    }

    RONIN_INLINE void SetBit(uint32 index) { ((uint8*)mUpdateMask)[ index >> 3 ] |= 1 << (index & 0x7); }
    RONIN_INLINE void UnsetBit(uint32 index) { ((uint8*)mUpdateMask)[ index >> 3 ] &= (0xff ^ (1 << (index & 0x7))); }
    RONIN_INLINE bool GetBit(uint32 index) const { return (((uint8*)mUpdateMask)[ index >> 3 ] & (1 << (index & 0x7))) != 0; }

    RONIN_INLINE uint32 GetBlockCount() const { return mBlocks; }
    RONIN_INLINE uint32 GetLength() const { return mBlocks << 2; }
    RONIN_INLINE uint32 GetCount() const { return mCount; }
    RONIN_INLINE uint8* GetMask() { return (uint8*)mUpdateMask; }

    RONIN_INLINE void SetCount(uint32 valuesCount)
    {
        if(mUpdateMask)
            delete[] mUpdateMask;

        mCount = valuesCount;
        mBlocks = (valuesCount + 31) / 32;

        mUpdateMask = new uint32[mBlocks];
        memset(mUpdateMask, 0, GetLength());
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
    uint32* mUpdateMask;
};
