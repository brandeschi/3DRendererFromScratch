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

typedef float f32;
typedef double f64;

#define global static
#define neo_assert(expression) if (!(expression)) { *(int *)0 = 0; }
#define arr_count(array) (sizeof(array) / sizeof((array)[0]))

// Sub TU
#include "math.cpp"
#include "array.cpp"
#include "mesh.cpp"

