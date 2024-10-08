#pragma once

// C headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL.h>

// Type Definitions!
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t mem_index;
typedef mem_index ums;

typedef float f32;
typedef double f64;

#define global static
#define neo_assert(expression) if (!(expression)) { *(int *)0 = 0; }
#define arr_count(array) (sizeof((array)) / sizeof((array)[0]))

// Sub TU
#include "math.cpp"
#include "array.cpp"
#include "upng.cpp"
#include "mesh.cpp"
#include "texture.cpp"

// Types
struct camera {
  v3 position;
  v3 direction;
  f32 yaw;
};
struct light {
  v3 direction;
};
struct plane {
  v3 point;
  v3 normal;
};
#define MAX_POLY_VERTICES 10
#define MAX_TRIANGLES_FROM_POLYGON 10
struct polygon {
  v2 uvs[MAX_POLY_VERTICES];
  v3 vertices[MAX_POLY_VERTICES];
  i32 number_of_vertices;
};
enum ViewFrustumPlanesEnum {
  FP_LEFT,
  FP_RIGHT,
  FP_TOP,
  FP_BOTTOM,
  FP_NEAR,
  FP_FAR
};
