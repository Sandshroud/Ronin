/***
 * Demonstrike Core
 */

#pragma once

///////////////////////////////////////////////////////////
// Location vector class (X, Y, Z, O)
//////////////////////////////////////////////////////////
class SERVER_DECL LocationVector
{
public:
    // Constructors
    LocationVector(float X, float Y, float Z) : x(X), y(Y), z(Z), o(0) {}
    LocationVector(float X, float Y, float Z, float O) : x(X), y(Y), z(Z), o(O) {}
    LocationVector() : x(0), y(0), z(0), o(0) {}

    // (dx * dx + dy * dy + dz * dz)
    float DistanceSq(const LocationVector & comp)
    {
        float delta_x = RONIN_UTIL::Diff(comp.x, x);//fabs(comp.x - x);
        float delta_y = RONIN_UTIL::Diff(comp.y, y);//fabs(comp.y - y);
        float delta_z = RONIN_UTIL::Diff(comp.z, z);//fabs(comp.z - z);

        return (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
    }

    float DistanceSq(const float &X, const float &Y, const float &Z)
    {
        float delta_x = RONIN_UTIL::Diff(X, x);
        float delta_y = RONIN_UTIL::Diff(Y, y);
        float delta_z = RONIN_UTIL::Diff(Z, z);

        return (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
    }

    float Distance2DSq(const LocationVector & comp)
    {
        float delta_x = RONIN_UTIL::Diff(comp.x, x);
        float delta_y = RONIN_UTIL::Diff(comp.y, y);
        return (delta_x*delta_x + delta_y*delta_y);
    }

    float Distance2DSq(const float & X, const float & Y)
    {
        float delta_x = RONIN_UTIL::Diff(X, x);
        float delta_y = RONIN_UTIL::Diff(Y, y);
        return (delta_x*delta_x + delta_y*delta_y);
    }

    // atan2(dx / dy)
    float CalcAngTo(const LocationVector & dest)
    {
        float dx = dest.x - x;
        float dy = dest.y - y;
        if(dy != 0.0f)
            return atan2(dy, dx);
        return 0.0f;
    }

    float CalcAngFrom(const LocationVector & src)
    {
        float dx = x - src.x;
        float dy = y - src.y;
        if(dy != 0.0f)
            return atan2(dy, dx);
        return 0.0f;
    }

    void ChangeCoords(float X, float Y, float Z, float O = 0.f)
    {
        x = X;
        y = Y;
        z = Z;
        o = O;
    }

    // add/subtract/equality vectors
    LocationVector & operator += (const LocationVector & add)
    {
        x += add.x;
        y += add.y;
        z += add.z;
        o += add.o;
        return *this;
    }

    LocationVector & operator -= (const LocationVector & sub)
    {
        x -= sub.x;
        y -= sub.y;
        z -= sub.z;
        o -= sub.o;
        return *this;
    }

    LocationVector & operator = (const LocationVector & eq)
    {
        x = eq.x;
        y = eq.y;
        z = eq.z;
        o = eq.o;
        return *this;
    }

    bool operator == (const LocationVector & eq)
    {
        if(eq.x == x && eq.y == y && eq.z == z)
            return true;
        return false;
    }

    bool operator != (const LocationVector & eq)
    {
        return !(*this == eq);
    }

    float x;
    float y;
    float z;
    float o;
};
