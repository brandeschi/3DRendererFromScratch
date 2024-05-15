#include "main.unity.h"

global b32 AppRunning = false;
global const u32 WIN_WIDTH = 1200;
global const u32 WIN_HEIGHT = 900;
global const u32 CUBE_DIMS = 9*9*9;
global const f32 FOV_FACTOR = 360.0f;

// TODO: Try out these with turns?
static v3 V3RotateX(v3 InitialVector, f32 Angle) {
  v3 Result = {
    InitialVector.x,
    InitialVector.y*cosf(Angle) - InitialVector.z*sinf(Angle),
    InitialVector.z*cosf(Angle) + InitialVector.y*sinf(Angle)
  };

  return Result;
}
static v3 V3RotateY(v3 InitialVector, f32 Angle) {
  v3 Result = {
    InitialVector.x*cosf(Angle) - InitialVector.z*sinf(Angle),
    InitialVector.y,
    InitialVector.z*cosf(Angle) + InitialVector.x*sinf(Angle)
  };

  return Result;
}
static v3 V3RotateZ(v3 InitialVector, f32 Angle) {
  v3 Result = {
    InitialVector.x*cosf(Angle) - InitialVector.y*sinf(Angle),
    InitialVector.y*cosf(Angle) + InitialVector.x*sinf(Angle),
    InitialVector.z
  };

  return Result;
}

static void DrawRect(u32 *ColorBuffer, u32 x, u32 y, u32 w, u32 h, u32 Color) {
  neo_assert(x >= 0 && x + w <= WIN_WIDTH);
  neo_assert(y >= 0 && y + h <= WIN_HEIGHT);

  for (u32 RectRow = y; RectRow < (y + h); ++RectRow) {
    for (u32 RectCol = x; RectCol < (x + w); ++RectCol) {
      ColorBuffer[(RectRow*WIN_WIDTH) + RectCol] = Color;
    }
  }
}

int main(int argc, char** argv) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initing SDL2\n");
    return -1;
  }

#if 0
  SDL_DisplayMode display_mode;
  if (SDL_GetCurrentDisplayMode(0, &display_mode) != 0) {
    fprintf(stderr, "Error getting display\n");
    return -1;
  }
  WIN_WIDTH = display_mode.w;
  WIN_HEIGHT = display_mode.h;
#endif

  SDL_Window *Window = SDL_CreateWindow("3D Renderer From Scratch",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WIN_WIDTH, WIN_HEIGHT,
                                        0);
  if (!Window) {
    fprintf(stderr, "Error creating SDL2 window\n");
    return -1;
  }

  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  if (!Renderer) {
    fprintf(stderr, "Error creating SDL2 renderer\n");
    return -1;
  }
  // NOTE: Turning this off due to screen flash switching display mode to full screen everytime
  // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

  // SETUP
  u32 *ColorBuff = (u32 *)malloc(sizeof(u32)*WIN_WIDTH*WIN_HEIGHT);
  if (!ColorBuff) {
    fprintf(stderr, "Error allocating with malloc\n");
    return -1;
  }
  SDL_Texture *CBTexture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH, WIN_HEIGHT);

  v3 CameraPos = { 0.0f, 0.0f, -5.0f };
  v3 CubeRotation = { 0.0f, 0.0f, 0.0f };
  // 9x9x9 Cube
  u32 PointCount = 0;
  v3 CubePoints[CUBE_DIMS];
  for (f32 x = -1.0f; x <= 1.0f; x += 0.25f) {
    for (f32 y = -1.0f; y <= 1.0f; y += 0.25f) {
      for (f32 z = -1.0f; z <= 1.0f; z += 0.25f) {
        v3 CubePoint = { x, y, z };
        CubePoints[PointCount++] = CubePoint;
      }
    }
  }

  v2 ProjectedPoints[CUBE_DIMS];
  AppRunning = true;
  while (AppRunning) {
    // INPUT
    SDL_Event Event;
    SDL_PollEvent(&Event);

    switch (Event.type) {
      case SDL_QUIT: {
        AppRunning = false;
      } break;
      case SDL_KEYDOWN: {
        if(Event.key.keysym.sym == SDLK_ESCAPE) {
          AppRunning = false;
        }
      } break;
    }

    // UPDATE

    CubeRotation.y += 0.001f;
    CubeRotation.z += 0.001f;
    // 'project' 3D points to 2D
    for (u32 i = 0; i < CUBE_DIMS; ++i) {
      v3 NewPoint = V3RotateY(CubePoints[i], CubeRotation.y);
      NewPoint = V3RotateZ(NewPoint, CubeRotation.z);
      NewPoint.z -= CameraPos.z;
      ProjectedPoints[i] = { (NewPoint.x*FOV_FACTOR) / NewPoint.z, (NewPoint.y*FOV_FACTOR) / NewPoint.z };
    }

    // RENDER

    // Clear
    for (u32 y = 0; y < WIN_HEIGHT; ++y) {
      for (u32 x = 0; x < WIN_WIDTH; ++x) {
          ColorBuff[(WIN_WIDTH * y) + x] = 0xFF616E8B;
      }
    }
    // Dot Matrix
    for (u32 y = 0; y < WIN_HEIGHT; y += 10) {
      for (u32 x = 0; x < WIN_WIDTH; x += 10) {
        if (y == 0 || x == 0) {
          continue;
        }
          ColorBuff[(WIN_WIDTH * y) + x] = 0xFFA0A8B9;
      }
    }

    for (u32 i = 0; i < CUBE_DIMS; ++i) {
      DrawRect(ColorBuff, (u32)(ProjectedPoints[i].x + (f32)(WIN_WIDTH / 2)), (u32)(ProjectedPoints[i].y + (f32)(WIN_HEIGHT / 2)), 5, 5, 0xFF00FF00);
    }

    SDL_UpdateTexture(CBTexture, 0, ColorBuff, (int)(WIN_WIDTH*sizeof(u32)));
    SDL_RenderCopy(Renderer, CBTexture, 0, 0);
    SDL_RenderPresent(Renderer);
  }

  return 0;
}
