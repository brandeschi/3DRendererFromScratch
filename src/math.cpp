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

inline f32 V2Length(v2 Vector) {
  f32 Result = 0.0f;
  Result = sqrtf((Vector.x*Vector.x) + (Vector.y*Vector.y));
  return Result;
}

inline void V2Normalize(v2 *Vector) {
  f32 VectorLength = V2Length(*Vector);
  Vector->x = Vector->x / VectorLength;
  Vector->y = Vector->y / VectorLength;
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

inline f32 V3Length(v3 Vector) {
  f32 Result = 0.0f;
  Result = sqrtf((Vector.x*Vector.x) + (Vector.y*Vector.y) + (Vector.z*Vector.z));
  return Result;
}

inline void V3Normalize(v3 *Vector) {
  f32 VectorLength = V3Length(*Vector);
  Vector->x = Vector->x / VectorLength;
  Vector->y = Vector->y / VectorLength;
  Vector->z = Vector->z / VectorLength;
}

// TODO: Try out these with turns?
v3 V3RotateX(v3 InitialVector, f32 Angle) {
  v3 Result = {
    InitialVector.x,
    InitialVector.y*cosf(Angle) - InitialVector.z*sinf(Angle),
    InitialVector.z*cosf(Angle) + InitialVector.y*sinf(Angle)
  };

  return Result;
}
v3 V3RotateY(v3 InitialVector, f32 Angle) {
  v3 Result = {
    InitialVector.x*cosf(Angle) - InitialVector.z*sinf(Angle),
    InitialVector.y,
    InitialVector.z*cosf(Angle) + InitialVector.x*sinf(Angle)
  };

  return Result;
}
v3 V3RotateZ(v3 InitialVector, f32 Angle) {
  v3 Result = {
    InitialVector.x*cosf(Angle) - InitialVector.y*sinf(Angle),
    InitialVector.y*cosf(Angle) + InitialVector.x*sinf(Angle),
    InitialVector.z
  };

  return Result;
}

// V4
inline v4 V4(f32 x, f32 y, f32 z, f32 w)
{
  v4 result;
  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;
  return result;
}

// Vector Casting
inline v4 V4FromV3(v3 Vector) {
  v4 Result = {0};
  Result.x = Vector.x;
  Result.y = Vector.y;
  Result.z = Vector.z;
  Result.w = 1.0f;
  return Result;
}

inline v3 V3FromV4(v4 Vector) {
  v3 Result = {0};
  Result.x = Vector.x;
  Result.y = Vector.y;
  Result.z = Vector.z;
  return Result;
}

// Matrices
union mat4
{
  struct {
    f32 m0,  m1,  m2,  m3;
    f32 m4,  m5,  m6,  m7;
    f32 m8,  m9,  m10, m11;
    f32 m12, m13, m14, m15;
  };

  f32 e[16];
};

inline mat4 Mat4Iden()
{
  mat4 Result = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return Result;
}

inline mat4 Mat4Multiply(mat4 Left, mat4 Right)
{
  mat4 Result = {};

  Result.m0 =  Left.m0*Right.m0 + Left.m1*Right.m4 + Left.m2*Right.m8 + Left.m3*Right.m12;
  Result.m1 =  Left.m0*Right.m1 + Left.m1*Right.m5 + Left.m2*Right.m9 + Left.m3*Right.m13;
  Result.m2 =  Left.m0*Right.m2 + Left.m1*Right.m6 + Left.m2*Right.m10 + Left.m3*Right.m14;
  Result.m3 =  Left.m0*Right.m3 + Left.m1*Right.m7 + Left.m2*Right.m11 + Left.m3*Right.m15;
  Result.m4 =  Left.m4*Right.m0 + Left.m5*Right.m4 + Left.m6*Right.m8 + Left.m7*Right.m12;
  Result.m5 =  Left.m4*Right.m1 + Left.m5*Right.m5 + Left.m6*Right.m9 + Left.m7*Right.m13;
  Result.m6 =  Left.m4*Right.m2 + Left.m5*Right.m6 + Left.m6*Right.m10 + Left.m7*Right.m14;
  Result.m7 =  Left.m4*Right.m3 + Left.m5*Right.m7 + Left.m6*Right.m11 + Left.m7*Right.m15;
  Result.m8 =  Left.m8*Right.m0 + Left.m9*Right.m4 + Left.m10*Right.m8 + Left.m11*Right.m12;
  Result.m9 =  Left.m8*Right.m1 + Left.m9*Right.m5 + Left.m10*Right.m9 + Left.m11*Right.m13;
  Result.m10 = Left.m8*Right.m2 + Left.m9*Right.m6 + Left.m10*Right.m10 + Left.m11*Right.m14;
  Result.m11 = Left.m8*Right.m3 + Left.m9*Right.m7 + Left.m10*Right.m11 + Left.m11*Right.m15;
  Result.m12 = Left.m12*Right.m0 + Left.m13*Right.m4 + Left.m14*Right.m8 + Left.m15*Right.m12;
  Result.m13 = Left.m12*Right.m1 + Left.m13*Right.m5 + Left.m14*Right.m9 + Left.m15*Right.m13;
  Result.m14 = Left.m12*Right.m2 + Left.m13*Right.m6 + Left.m14*Right.m10 + Left.m15*Right.m14;
  Result.m15 = Left.m12*Right.m3 + Left.m13*Right.m7 + Left.m14*Right.m11 + Left.m15*Right.m15;

  return Result;
}

inline mat4 operator*(mat4 Left, mat4 Right)
{
  mat4 Result = {0};
  Result = Mat4Multiply(Left, Right);

  return Result;
}

inline mat4 &operator*=(mat4 &a, mat4 b)
{
    a = a*b;
    return a;
}

inline mat4 Mat4Translate(f32 x, f32 y, f32 z)
{
  mat4 Result = {
    1.0f, 0.0f, 0.0f, x,
    0.0f, 1.0f, 0.0f, y,
    0.0f, 0.0f, 1.0f, z,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return Result;
}

inline mat4 Mat4Scale(f32 x, f32 y, f32 z)
{
  mat4 Result = {
    x, 0.0f, 0.0f, 0.0f,
    0.0f, y, 0.0f, 0.0f,
    0.0f, 0.0f, z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return Result;
}

// Left-Handed Rotation Matrices
inline mat4 Mat4RotateX(f32 Angle)
{
  mat4 Result = {
    1.0f,   0.0f,         0.0f,         0.0f,
    0.0f,   cosf(Angle),  sinf(Angle), 0.0f,
    0.0f,   -sinf(Angle), cosf(Angle),  0.0f,
    0.0f,   0.0f,         0.0f,         1.0f
  };

  return Result;
}

inline mat4 Mat4RotateY(f32 Angle)
{
  mat4 Result = {
    cosf(Angle),  0.0f, -sinf(Angle), 0.0f,
    0.0f,         1.0f, 0.0f,        0.0f,
    sinf(Angle),  0.0f, cosf(Angle), 0.0f,
    0.0f,         0.0f, 0.0f,        1.0f
  };

  return Result;
}

inline mat4 Mat4RotateZ(f32 Angle)
{
  mat4 Result = {
    cosf(Angle),  sinf(Angle),  0.0f, 0.0f,
    -sinf(Angle), cosf(Angle),  0.0f, 0.0f,
    0.0f,         0.0f,         1.0f, 0.0f,
    0.0f,         0.0f,         0.0f, 1.0f
  };

  return Result;
}

inline mat4 Mat4Ortho(f32 Left, f32 Right, f32 Top, f32 Bottom, f32 NearP, f32 FarP)
{
  f32 RlRange = Right - Left;
  f32 TbRange = Top - Bottom;
  f32 FnRange = FarP - NearP;

  // NOTE: This is colunm-major(OGL inforced); which means the rows in these map to columns in math notation.
  mat4 Result = {
    (2.0f/RlRange),              0.0f,                         0.0f,                       0.0f,
    0.0f,                         (2.0f/TbRange),              0.0f,                       0.0f,
    0.0f,                         0.0f,                         (-2.0f/FnRange),           0.0f,
    -((Right + Left)/(RlRange)), -((Top + Bottom)/(TbRange)), -((FarP + NearP)/(FnRange)), 1.0f
  };

  return Result;
}

inline mat4 Mat4Projection(f32 AspectRatio, f32 FOV, f32 ZNear, f32 ZFar)
{
  f32 FOVFunc = (1.0f / tanf((FOV / 2.0f)));
  f32 NormRatio = (ZFar / (ZFar - ZNear));
  mat4 Result = {
    AspectRatio*FOVFunc, 0.0f,    0.0f,      0.0f,
    0.0f,                FOVFunc, 0.0f,      0.0f,
    0.0f,                0.0f,    NormRatio, -NormRatio*ZNear,
    0.0f,                0.0f,    1.0f,      0.0f
  };
  return Result;
}

mat4 M4LookAt(v3 Eye, v3 Target, v3 Up) {
  v3 Z = Target - Eye;
  V3Normalize(&Z);
  v3 X = CrossProduct(Up, Z);
  V3Normalize(&X);
  v3 Y = CrossProduct(Z, X);

  mat4 Result = {
    X.x,  X.y,  X.z,  -DotProduct(X, Eye),
    Y.x,  Y.y,  Y.z,  -DotProduct(Y, Eye),
    Z.x,  Z.y,  Z.z,  -DotProduct(Z, Eye),
    0.0f, 0.0f, 0.0f, 1.0f,
  };

  return Result;
}

// Mixed Funcs
inline v4 Mat4MultV4(mat4 Matrix, v4 Vector) {
  v4 Result = {0};
  Result.x = (Matrix.m0*Vector.x) + (Matrix.m1*Vector.y) + (Matrix.m2*Vector.z) + (Matrix.m3*Vector.w);
  Result.y = (Matrix.m4*Vector.x) + (Matrix.m5*Vector.y) + (Matrix.m6*Vector.z) + (Matrix.m7*Vector.w);
  Result.z = (Matrix.m8*Vector.x) + (Matrix.m9*Vector.y) + (Matrix.m10*Vector.z) + (Matrix.m11*Vector.w);
  Result.w = (Matrix.m12*Vector.x) + (Matrix.m13*Vector.y) + (Matrix.m14*Vector.z) + (Matrix.m15*Vector.w);
  return Result;
}

inline v4 operator*(mat4 Left, v4 Right)
{
  v4 Result = {0};
  Result = Mat4MultV4(Left, Right);
  return Result;
}

