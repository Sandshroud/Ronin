/***
 * Demonstrike Core
 */

#pragma once

const int INT24_MAX = 8388607;

class SERVER_DECL int24
{
protected:
    uint8 blocks[3];

public:
    int24()
    {

    }

    int24( const int val )
    {
        *this = val;
    }

    int24( const int24& val )
    {
        *this = val;
    }

    operator int() const
    {
        if ( blocks[2] & 0x80 )
        {
            return (0xff << 24) | (blocks[2] << 16) | (blocks[1] << 8) | (blocks[0] << 0);
        }
        else
        {
            return (blocks[2] << 16) | (blocks[1] << 8) | (blocks[0] << 0);
        }
    }

    operator float() const
    {
        return (float)this->operator int();
    }

    int24& operator =( const int24& input )
    {
        blocks[0] = input.blocks[0];
        blocks[1] = input.blocks[1];
        blocks[2] = input.blocks[2];
        return *this;
    }

    int24& operator =( const int input )
    {
        blocks[0] = ((unsigned char*)&input)[0];
        blocks[1] = ((unsigned char*)&input)[1];
        blocks[2] = ((unsigned char*)&input)[2];
        return *this;
    }

    int24 operator +( const int24& val ) const
    {
        return int24( (int)*this + (int)val );
    }

    int24 operator -( const int24& val ) const
    {
        return int24( (int)*this - (int)val );
    }

    int24 operator *( const int24& val ) const
    {
        return int24( (int)*this * (int)val );
    }

    int24 operator /( const int24& val ) const
    {
        return int24( (int)*this / (int)val );
    }

    int24 operator +( const int val ) const
    {
        return int24( (int)*this + val );
    }

    int24 operator -( const int val ) const
    {
        return int24( (int)*this - val );
    }

    int24 operator *( const int val ) const
    {
        return int24( (int)*this * val );
    }

    int24 operator /( const int val ) const
    {
        return int24( (int)*this / val );
    }

    int24& operator +=( const int24& val )
    {
        *this = *this + val;
        return *this;
    }

    int24& operator -=( const int24& val )
    {
        *this = *this - val;
        return *this;
    }

    int24& operator *=( const int24& val )
    {
        *this = *this * val;
        return *this;
    }

    int24& operator /=( const int24& val )
    {
        *this = *this / val;
        return *this;
    }

    int24& operator +=( const int val )
    {
        *this = *this + val;
        return *this;
    }

    int24& operator -=( const int val )
    {
        *this = *this - val;
        return *this;
    }

    int24& operator *=( const int val )
    {
        *this = *this * val;
        return *this;
    }

    int24& operator /=( const int val )
    {
        *this = *this / val;
        return *this;
    }

    int24 operator >>( const int val ) const
    {
        return int24( (int)*this >> val );
    }

    int24 operator <<( const int val ) const
    {
        return int24( (int)*this << val );
    }

    int24& operator >>=( const int val )
    {
        *this = *this >> val;
        return *this;
    }

    int24& operator <<=( const int val )
    {
        *this = *this << val;
        return *this;
    }

    operator bool() const
    {
        return (int)*this != 0;
    }

    bool operator !() const
    {
        return !((int)*this);
    }

    int24 operator -()
    {
        return int24( -(int)*this );
    }

    bool operator ==( const int24& val ) const
    {
        return (int)*this == (int)val;
    }

    bool operator !=( const int24& val ) const
    {
        return (int)*this != (int)val;
    }

    bool operator >=( const int24& val ) const
    {
        return (int)*this >= (int)val;
    }

    bool operator <=( const int24& val ) const
    {
        return (int)*this <= (int)val;
    }

    bool operator >( const int24& val ) const
    {
        return (int)*this > (int)val;
    }

    bool operator <( const int24& val ) const
    {
        return (int)*this < (int)val;
    }

    bool operator ==( const int val ) const
    {
        return (int)*this == val;
    }

    bool operator !=( const int val ) const
    {
        return (int)*this != val;
    }

    bool operator >=( const int val ) const
    {
        return (int)*this >= val;
    }

    bool operator <=( const int val ) const
    {
        return (int)*this <= val;
    }

    bool operator >( const int val ) const
    {
        return ((int)*this) > val;
    }

    bool operator <( const int val ) const
    {
        return (int)*this < val;
    }
};
