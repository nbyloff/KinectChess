// vector.h

#ifndef VECTOR_H
#define VECTOR_H
#include <cmath>

class Vector
{
public:
    Vector(float X = 0.f, float Y = 0.f, float Z = 0.f)
    : x(X), y(Y), z(Z) {}

    // Vector Operators
    inline Vector operator +(const Vector &v)
    { return Vector(x + v.x, y + v.y, z + v.z); }
    inline Vector operator -(const Vector &v)
    { return Vector(x - v.x, y - v.y, z - v.z); }
    inline Vector operator *(const Vector &v)
    { return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }

    inline Vector operator +=(const Vector &v)
    { return Vector(x += v.x, y += v.y, z += v.z); }
    inline Vector operator -=(const Vector &v)
    { return Vector(x -= v.x, y -= v.y, z -= v.z); }
    inline Vector operator *=(const Vector &v)
    { return *this = Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }

    // Scaler operators
    inline Vector operator *(const float f)
    { return Vector(x*f, y*f, z*f); }
    inline Vector operator /(const float f)
    { return (*this)*(1.f/f); }

    union
    {
        struct
        {
            float x, y, z;
        };
        float v[3];
    };

};

#endif
