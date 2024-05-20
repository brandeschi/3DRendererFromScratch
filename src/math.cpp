#pragma once
#include "main.unity.h"
#include "math.h"

// Math helpers
inline f32 Squared(f32 input)
{
    f32 result = input * input;
    return result;
}

// V2
inline v2 V2(f32 x, f32 y)
{
    v2 result;
    result.x = x;
    result.y = y;
    return result;
}

inline v2 operator*(f32 real, v2 a)
{
    v2 result;
    result.x = real * a.x;
    result.y = real * a.y;
    return result;
}

inline v2 operator*(v2 a, f32 real)
{
    v2 result = real * a;
    return result;
}

inline v2 &operator*=(v2 &a, f32 real)
{
    a = real * a;
    return a;
}

inline v2 operator/(f32 real, v2 a)
{
    v2 result;
    result.x = a.x / real;
    result.y = a.y / real;
    return result;
}

inline v2 operator/(v2 a, f32 real)
{
    v2 result = a / real;
    return result;
}

inline v2 &operator/=(v2 &a, f32 real)
{
    a = a / real;
    return a;
}

inline v2 operator-(v2 a)
{
    v2 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

inline v2 operator+(v2 a, v2 b)
{
    v2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline v2 &operator+=(v2 &a, v2 b)
{
    a = a + b;
    return a;
}

inline v2 operator-(v2 a, v2 b)
{
    v2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline f32 DotProduct(v2 a, v2 b)
{
    f32 result = a.x*b.x + a.y*b.y;
    return result;
}

// inline f32 length_sq(v2 a)
// {
//     f32 result = dot_product(a, a);
//     return result;
// }

inline f32 Vector2Length(v2 Vector) {
  f32 Result = 0.0f;
  Result = sqrtf((Vector.x*Vector.x) + (Vector.y*Vector.y));
  return Result;
}

// V3
inline v3 V3(f32 x, f32 y, f32 z)
{
    v3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

inline v3 operator*(f32 real, v3 a)
{
    v3 result;
    result.x = real * a.x;
    result.y = real * a.y;
    result.z = real * a.z;
    return result;
}

inline v3 operator*(v3 a, f32 real)
{
    v3 result = real * a;
    return result;
}

inline v3 &operator*=(v3 &a, f32 real)
{
    a = real * a;
    return a;
}

inline v3 operator/(f32 real, v3 a)
{
    v3 result;
    result.x = a.x / real;
    result.y = a.y / real;
    result.z = a.z / real;
    return result;
}

inline v3 operator/(v3 a, f32 real)
{
    v3 result = a / real;
    return result;
}

inline v3 &operator/=(v3 &a, f32 real)
{
    a = a / real;
    return a;
}

inline v3 operator-(v3 a)
{
    v3 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    return result;
}

inline v3 operator+(v3 a, v3 b)
{
    v3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline v3 &operator+=(v3 &a, v3 b)
{
    a = a + b;
    return a;
}

inline v3 operator-(v3 a, v3 b)
{
    v3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline f32 DotProduct(v3 a, v3 b)
{
    f32 result = (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
    return result;
}

// inline f32 length_sq(v2 a)
// {
//     f32 result = dot_product(a, a);
//     return result;
// }

inline v3 CrossProduct(v3 a, v3 b) {
  v3 Result = {0};
  Result.x = (a.y*b.z) - (a.z*b.y);
  Result.y = (a.z*b.x) - (a.x*b.z);
  Result.z = (a.x*b.y) - (a.y*b.x);
  return Result;
}

inline f32 Vector3Length(v3 Vector) {
  f32 Result = 0.0f;
  Result = sqrtf((Vector.x*Vector.x) + (Vector.y*Vector.y) + (Vector.z*Vector.z));
  return Result;
}
