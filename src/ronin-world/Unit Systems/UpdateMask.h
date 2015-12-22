/***
 * Demonstrike Core
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

    ~UpdateMask() { delete[] mUpdateMask; }

    RONIN_INLINE void SetBit(uint32 index) { mUpdateMask[index>>3] |= 1 << (index & 0x7); }
    RONIN_INLINE void UnsetBit(uint32 index) { mUpdateMask[index>>3] &= ~(1 << (index & 0x7)); }
    RONIN_INLINE bool GetBit(uint32 index) const { return (mUpdateMask[index>>3] & (1 << (index & 0x7))) != 0; }

    RONIN_INLINE uint32 GetBlockCount() const { return mBlocks>>2; }
    RONIN_INLINE uint32 GetLength() const { return mBlocks; }
    RONIN_INLINE uint32 GetCount() const { return mCount; }
    RONIN_INLINE uint8* GetMask() { return mUpdateMask; }

    RONIN_INLINE uint8 GetBlock(uint32 index) { return mUpdateMask[index]; }
    RONIN_INLINE void SetBlock(uint32 index, uint8 value) { mUpdateMask[index] = value; }

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
