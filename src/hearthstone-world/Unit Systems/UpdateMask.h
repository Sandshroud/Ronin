/***
 * Demonstrike Core
 */

#pragma once

class UpdateMask
{
    uint32 *m_mask;
    uint32 mCount; // in values
    uint32 mBlocks; // in uint32 blocks

public:
    UpdateMask( ) : m_mask( 0 ), mCount( 0 ), mBlocks( 0 ) { }
    UpdateMask( const UpdateMask& mask ) : m_mask( 0 ) { *this = mask; }

    uint32 AByte(const uint32 index) { return (uint32)floor(float(index)/32.0f); }
    uint8 ABit(const uint32 index) { return index%32; }

    ~UpdateMask( )
    {
        if(m_mask)
            delete [] m_mask;
    }

    void SetBit( const uint32 index )
    {
        ASSERT(index < mCount);
        m_mask[AByte(index)] |= uint32(uint32(1) << ABit(index));
    }

    void UnsetBit( const uint32 index )
    {
        ASSERT(index < mCount);
        m_mask[AByte(index)] &= ~uint32(uint32(1) << ABit(index));
    }

    bool GetBit( const uint32 index )
    {
        ASSERT(index < mCount);
        return (m_mask[AByte(index)] & uint32(uint32(1) << ABit(index)));
    }

    HEARTHSTONE_INLINE uint32 GetLength() const { return (mBlocks*sizeof(uint32)); }
    HEARTHSTONE_INLINE uint32 GetBlockCount() const {return mBlocks;}
    HEARTHSTONE_INLINE uint32 GetCount() const { return mCount; }
    HEARTHSTONE_INLINE const uint8* GetMask() const { return (uint8*)m_mask; }

    uint32 GetUpdateBlockCount() const
    {
        for(uint32 x = mBlocks-1; x >= 0; x--)
        { if(m_mask[x]) return x+1; }
        return 1;
    }

    void SetCount(uint32 valuesCount)
    {
        if(m_mask)
            delete [] m_mask;

        mCount = valuesCount;
        mBlocks = (valuesCount + 31) / 32;

        m_mask = new uint32[mBlocks];
        memset(m_mask, 0, mBlocks*sizeof(uint32));
    }

    void Clear() { if (m_mask) memset(m_mask, 0, mBlocks*sizeof(uint32)); }
    UpdateMask& operator = ( const UpdateMask& mask )
    {
        SetCount(mask.mCount);
        memcpy(m_mask, mask.m_mask, mBlocks*sizeof(uint32));
        return *this;
    }

    void operator &= ( const UpdateMask& mask )
    {
        ASSERT(mask.mCount <= mCount);
        for(uint32 i = 0; i < mBlocks; i++)
            m_mask[i] &= mask.m_mask[i];
    }

    void operator |= ( const UpdateMask& mask )
    {
        ASSERT(mask.mCount <= mCount);
        for(uint32 i = 0; i < mBlocks; i++)
            m_mask[i] |= mask.m_mask[i];
    }

    UpdateMask operator & ( const UpdateMask& mask ) const
    {
        ASSERT(mask.mCount <= mCount);

        UpdateMask newmask;
        newmask = *this;
        newmask &= mask;
        return newmask;
    }

    UpdateMask operator | ( const UpdateMask& mask ) const
    {
        ASSERT(mask.mCount <= mCount);

        UpdateMask newmask;
        newmask = *this;
        newmask |= mask;
        return newmask;
    }
};
