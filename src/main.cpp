#include "main.unity.h"

triangle *Triangles = 0;
global mesh Mesh = {0};

global b32 AppRunning = false;
global u32 PrevFrameTime = 0;
global i32 SyncTime = 0;
#define FPS 30
#define TARGET_FRAME_TIME (1000 / FPS)
global const u32 WIN_WIDTH = 1200;
global const u32 WIN_HEIGHT = 900;
global const f32 FOV_FACTOR = 720.0f;

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

static void DrawLine(u32 *ColorBuffer, i32 x0, i32 y0, i32 x1, i32 y1, u32 Color) {
  i32 DeltaX = x1 - x0;
  i32 DeltaY = y1 - y0;

  i32 SideLength = abs(DeltaX) >= abs(DeltaY) ? abs(DeltaX) : abs(DeltaY);
  f32 XIncrement = (f32)DeltaX / (f32)SideLength;
  f32 YIncrement = (f32)DeltaY / (f32)SideLength;

  f32 CurrentX = (f32)x0;
  f32 CurrentY = (f32)y0;
  for (i32 i = 0; i < SideLength; ++i) {
    ColorBuffer[(WIN_WIDTH*(u32)roundf(CurrentY)) + (u32)roundf(CurrentX)] = Color;
    CurrentX += XIncrement;
    CurrentY += YIncrement;
  }
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

  // obj_file CubeMesh = LoadObjFile("./assets/cube.obj");
  obj_file CubeMesh = LoadObjFile("./assets/f22.obj");
#define CUBE_VERTICES_COUNT 8
  v3 CubeVertices[CUBE_VERTICES_COUNT] = {
    { -1.0f, -1.0f, -1.0f }, // 1
    { -1.0f,  1.0f, -1.0f }, // 2
    {  1.0f,  1.0f, -1.0f }, // 3
    {  1.0f, -1.0f, -1.0f }, // 4
    {  1.0f,  1.0f,  1.0f }, // 5
    {  1.0f, -1.0f,  1.0f }, // 6
    { -1.0f,  1.0f,  1.0f }, // 7
    { -1.0f, -1.0f,  1.0f }, // 8
  };

#define CUBE_FACE_COUNT (6 * 2) // 6 faces; 2 triangles per face
face_index CubeFaces[CUBE_FACE_COUNT] = {
  // front
  { 1, 2, 3 }, { 1, 3, 4 },
  // right
  { 4, 3, 5 }, { 4, 5, 6 },
  // back
  { 6, 5, 7 }, { 6, 7, 8 },
  // left
  { 8, 7, 2 }, { 8, 2, 1 },
  // top
  { 2, 7, 5 }, { 2, 5, 3 },
  // bottom
  { 6, 8, 1 }, { 6, 1, 4 }
};

  v3 CameraPos = { 0.0f, 0.0f, -5.0f };
  // Load cube mesh data
  // for (u32 i = 0; i < CUBE_VERTICES_COUNT; ++i) {
  //   array_push(Mesh.vertices, v3, CubeVertices[i]);
  // }
  // for (u32 i = 0; i < CUBE_FACE_COUNT; ++i) {
  //   array_push(Mesh.faces, face_index, CubeFaces[i]);
  // }
  Mesh.vertices = CubeMesh.vertices;
  Mesh.faces = CubeMesh.faces;

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

    Mesh.rotation.x += 0.01f;
    // Mesh.rotation.y += 0.01f;
    // Mesh.rotation.z += 0.01f;

    // Cube Verts (8)
    Triangles = 0;
    for (i32 i = 0; i < array_length(Mesh.faces); ++i) {
      // Collect vertices of triangle for each face
      v3 FaceVerts[3];
      FaceVerts[0] = Mesh.vertices[Mesh.faces[i].a - 1];
      FaceVerts[1] = Mesh.vertices[Mesh.faces[i].b - 1];
      FaceVerts[2] = Mesh.vertices[Mesh.faces[i].c - 1];

      triangle CurrentTriangle = {0};
      // Projection work on each vertex of triangle
      for (u32 j = 0; j < arr_count(FaceVerts); ++j) {
        // v3 NewVert = V3RotateY(FaceVerts[j], Mesh.rotation.y);
        // NewVert = V3RotateZ(NewVert, Mesh.rotation.z);
        v3 NewVert = V3RotateX(FaceVerts[j], Mesh.rotation.x);
        NewVert.z -= CameraPos.z;

        v2 ProjectedPoint = { (NewVert.x*FOV_FACTOR) / NewVert.z, (NewVert.y*FOV_FACTOR) / NewVert.z };
        ProjectedPoint.x += (f32)(WIN_WIDTH / 2);
        ProjectedPoint.y += (f32)(WIN_HEIGHT / 2);
        CurrentTriangle.vertices[j] = ProjectedPoint;
      }

      array_push(Triangles, triangle, CurrentTriangle);
    }

    // RENDER

    // Clear
    for (u32 y = 0; y < WIN_HEIGHT; ++y) {
      for (u32 x = 0; x < WIN_WIDTH; ++x) {
        // ColorBuff[(WIN_WIDTH * y) + x] = 0xFF616E8B;
        // Darker Background
        ColorBuff[(WIN_WIDTH * y) + x] = 0xFF3A4253;
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

    // Triangle vertices for each face of the mesh
    // At this stage, there are multiple overdraws of the vertices
    for (i32 i = 0; i < array_length(Triangles); ++i) {
      DrawRect(ColorBuff, (u32)Triangles[i].vertices[0].x, (u32)Triangles[i].vertices[0].y, 1, 1, 0xFF00FF00);
      DrawRect(ColorBuff, (u32)Triangles[i].vertices[1].x, (u32)Triangles[i].vertices[1].y, 1, 1, 0xFF00FF00);
      DrawRect(ColorBuff, (u32)Triangles[i].vertices[2].x, (u32)Triangles[i].vertices[2].y, 1, 1, 0xFF00FF00);
      DrawLine(ColorBuff, (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, 0xFF00FF00);
      DrawLine(ColorBuff, (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, 0xFF00FF00);
      DrawLine(ColorBuff, (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, 0xFF00FF00);
    }

    SDL_UpdateTexture(CBTexture, 0, ColorBuff, (int)(WIN_WIDTH*sizeof(u32)));
    SDL_RenderCopy(Renderer, CBTexture, 0, 0);
    SDL_RenderPresent(Renderer);

    array_free(Triangles);

    SyncTime = TARGET_FRAME_TIME - (SDL_GetTicks() - PrevFrameTime);
    if (SyncTime > 0) {
      SDL_Delay(SyncTime);
    }
    PrevFrameTime = SDL_GetTicks();
  }

  return 0;
}
