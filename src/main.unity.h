#pragma once
#define HAVE_LIBC

// C headers
#include <stdio.h>
#include <stdint.h>
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


// Sub TU
#include "mesh.cpp"

