#include "main.unity.h"

struct light {
  v3 direction;
};

inline u32 ColorFromLightIntensity(u32 Color, f32 Percentage) {
  // TODO: SafeTruncate??
  if (Percentage < 0.0f) Percentage = 0.0f;
  if (Percentage > 1.0f) Percentage = 1.0f;

  u32 A = (Color & 0xFF000000);
  u32 R = (u32)(((f32)(Color & 0x00FF0000))*Percentage);
  u32 G = (u32)(((f32)(Color & 0x0000FF00))*Percentage);
  u32 B = (u32)(((f32)(Color & 0x000000FF))*Percentage);

  u32 NewColor = A | (R & 0x00FF0000) | (G & 0x0000FF00) | (B & 0x000000FF);
  return NewColor;
}

triangle *Triangles = 0;
global mesh Mesh = {0};

// Per bit option for rendering mode
enum RenderModeEnum {
  WIREFRAME = 1 << 0,
  FILLED = 1 << 1,
  VERTICES = 1 << 2
};

global b32 AppRunning = false;
global u32 PrevFrameTime = 0;
global i32 SyncTime = 0;
global b32 BackFaceCull = true;
global u32 RenderMode = WIREFRAME | VERTICES;
#define FPS 30
#define TARGET_FRAME_TIME (1000 / FPS)
global const u32 WIN_WIDTH = 1200;
global const u32 WIN_HEIGHT = 900;
global const f32 FOV_FACTOR = 640.0f;

inline void SwapI32(i32 *a, i32 *b) {
  i32 temp = *a;
  *a = *b;
  *b = temp;
}
inline void SwapTriangles(triangle *a, triangle *b) {
  triangle temp = *a;
  *a = *b;
  *b = temp;
}

void BubbleSortTrianlges(triangle *triangles) {
  for (i32 i = 1; i < array_length(triangles); ++i) {
    for (i32 j = 0; j < array_length(triangles) - 1; ++j) {
      if (triangles[j].avg_depth < triangles[j + 1].avg_depth) { // < or > dependent on Coord system handness
        SwapTriangles(&triangles[j], &triangles[j + 1]);
      }
    }
  }
}

// Draw Funcs
static void DrawLine(u32 *ColorBuffer, i32 x0, i32 y0, i32 x1, i32 y1, u32 Color) {
  i32 DeltaX = x1 - x0;
  i32 DeltaY = y1 - y0;

  i32 SideLength = abs(DeltaX) >= abs(DeltaY) ? abs(DeltaX) : abs(DeltaY);
  f32 XIncrement = (f32)DeltaX / (f32)SideLength;
  f32 YIncrement = (f32)DeltaY / (f32)SideLength;

  f32 CurrentX = (f32)x0;
  f32 CurrentY = (f32)y0;
  for (i32 i = 0; i <= SideLength; ++i) {
    ColorBuffer[(WIN_WIDTH*(u32)roundf(CurrentY)) + (u32)roundf(CurrentX)] = Color;
    CurrentX += XIncrement;
    CurrentY += YIncrement;
  }
}

static void DrawFilledTriangle(u32 *ColorBuffer, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2, u32 Color) {
  // NOTE: This uses the Flat-top/Flat-bottom method
  // Ensure y's are sorted with y0 being the smallest and y2 being the largest
  if (y0 > y1) {
    SwapI32(&y0, &y1);
    SwapI32(&x0, &x1);
  }
  if (y1 > y2) {
    SwapI32(&y1, &y2);
    SwapI32(&x1, &x2);
  }
  if (y0 > y1) {
    SwapI32(&y0, &y1);
    SwapI32(&x0, &x1);
  }

  if (y0 == y1) {
    i32 DeltaX1X2 = x1 - x2;
    i32 DeltaY1Y2 = y1 - y2;
    i32 DeltaMXX2 = x0 - x2;
    i32 DeltaMYY2 = y0 - y2;
    f32 InvSlopeOneToTwo = (f32)DeltaX1X2 / (f32)DeltaY1Y2;
    f32 InvSlopeMToTwo = (f32)DeltaMXX2 / (f32)DeltaMYY2;
    f32 StartX = (f32)x1;
    f32 EndX = (f32)x0;
    for (i32 row = y1; row <= y2; ++row) {
      DrawLine(ColorBuffer, (i32)StartX, row, (i32)EndX, row, Color);
      StartX += InvSlopeOneToTwo;
      EndX += InvSlopeMToTwo;
    }
  } else if (y1 == y2) {
    i32 DeltaX1X0 = x1 - x0;
    i32 DeltaY1Y0 = y1 - y0;
    i32 DeltaX2X0 = x2 - x0;
    i32 DeltaY2Y0 = y2 - y0;
    f32 InvSlopeX1Y1 = (f32)DeltaX1X0 / (f32)DeltaY1Y0;
    f32 InvSlopeX2Y2 = (f32)DeltaX2X0 / (f32)DeltaY2Y0;

    f32 StartX = (f32)x0;
    f32 EndX = (f32)x0;
    for (i32 row = y0; row <= y1; ++row) {
      DrawLine(ColorBuffer, (i32)StartX, row, (i32)EndX, row, Color);
      StartX += InvSlopeX1Y1;
      EndX += InvSlopeX2Y2;
    }
  } else {
    i32 My = y1;
    i32 Mx = (i32)(((f32)((x2 - x0)*(y1 - y0)) / (f32)(y2 - y0)) + x0);

    // Flat-bottom
    // Since we will always move downward at a constant Y, we actually want to find the INVERSE slope of the line
    i32 DeltaX1X0 = x1 - x0;
    i32 DeltaY1Y0 = y1 - y0;
    i32 DeltaX2X0 = Mx - x0;
    i32 DeltaY2Y0 = My - y0;
    f32 InvSlopeX1Y1 = (f32)DeltaX1X0 / (f32)DeltaY1Y0;
    f32 InvSlopeX2Y2 = (f32)DeltaX2X0 / (f32)DeltaY2Y0;

    f32 StartX = (f32)x0;
    f32 EndX = (f32)x0;
    for (i32 row = y0; row <= My; ++row) {
      DrawLine(ColorBuffer, (i32)StartX, row, (i32)EndX, row, Color);
      StartX += InvSlopeX1Y1;
      EndX += InvSlopeX2Y2;
    }

    // Flat-top
    i32 DeltaX1X2 = x1 - x2;
    i32 DeltaY1Y2 = y1 - y2;
    i32 DeltaMXX2 = Mx - x2;
    i32 DeltaMYY2 = My - y2;
    f32 InvSlopeOneToTwo = (f32)DeltaX1X2 / (f32)DeltaY1Y2;
    f32 InvSlopeMToTwo = (f32)DeltaMXX2 / (f32)DeltaMYY2;
    StartX = (f32)x1;
    EndX = (f32)Mx;
    for (i32 row = y1; row <= y2; ++row) {
      DrawLine(ColorBuffer, (i32)StartX, row, (i32)EndX, row, Color);
      StartX += InvSlopeOneToTwo;
      EndX += InvSlopeMToTwo;
    }
  }
}

static void DrawRect(u32 *ColorBuffer, u32 x, u32 y, u32 w, u32 h, u32 Color) {
  if (!(x >= 0 && x + w <= WIN_WIDTH) && !(y >= 0 && y + h <= WIN_HEIGHT)) {
    return;
  }

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

  mesh CubeMesh = LoadMeshFromObjFile("./assets/cube.obj");
  // mesh CubeMesh = LoadMeshFromObjFile("./assets/f22.obj");
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
  { 1, 2, 3, 0xFFFFFFFF }, { 1, 3, 4, 0xFFFFFFFF },
  // right
  { 4, 3, 5, 0xFFFFFFFF }, { 4, 5, 6, 0xFFFFFFFF },
  // back
  { 6, 5, 7, 0xFFFFFFFF }, { 6, 7, 8, 0xFFFFFFFF },
  // left
  { 8, 7, 2, 0xFFFFFFFF }, { 8, 2, 1, 0xFFFFFFFF },
  // top
  { 2, 7, 5, 0xFFFFFFFF }, { 2, 5, 3, 0xFFFFFFFF },
  // bottom
  { 6, 8, 1, 0xFFFFFFFF }, { 6, 1, 4, 0xFFFFFFFF }
};

  v3 CameraPos = { 0.0f, 0.0f, 0.0f };
  // Load cube mesh data
  for (u32 i = 0; i < CUBE_VERTICES_COUNT; ++i) {
    array_push(Mesh.vertices, v3, CubeVertices[i]);
  }
  for (u32 i = 0; i < CUBE_FACE_COUNT; ++i) {
    array_push(Mesh.faces, face_index, CubeFaces[i]);
  }

  // Mesh.vertices = CubeMesh.vertices;
  // Mesh.faces = CubeMesh.faces;
  Mesh.scale = { 1.0f, 1.0f, 1.0f };
  f32 FOV = PI32 / 3.0f;
  f32 ZNear = 0.1f;
  f32 ZFar = 100.0f;
  mat4 ProjectionMatrix = Mat4Projection(((f32)WIN_HEIGHT / (f32)WIN_WIDTH), FOV, ZNear, ZFar);
  light GLight = { 0.0f, 0.0f, 1.0f };

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
        if(Event.key.keysym.sym == SDLK_1) {
          RenderMode = WIREFRAME | VERTICES;
        }
        if(Event.key.keysym.sym == SDLK_2) {
          RenderMode = WIREFRAME;
        }
        if(Event.key.keysym.sym == SDLK_3) {
          RenderMode = FILLED;
        }
        if(Event.key.keysym.sym == SDLK_4) {
          RenderMode = FILLED | WIREFRAME;
        }
        if(Event.key.keysym.sym == SDLK_c) {
          BackFaceCull = true;
        }
        if(Event.key.keysym.sym == SDLK_d) {
          BackFaceCull = false;
        }
      } break;
    }

    // UPDATE

    // Mesh.scale.x += 0.002f;

    Mesh.rotation.x += 0.02f;
    Mesh.rotation.y += 0.02f;
    Mesh.rotation.z += 0.02f;

    // Mesh.translation.x += 0.01f;
    Mesh.translation.z = 5.0f;

    mat4 ScaleMatrix = Mat4Scale(Mesh.scale.x, Mesh.scale.y, Mesh.scale.z);
    mat4 XRotationMatrix = Mat4RotateX(Mesh.rotation.x);
    mat4 YRotationMatrix = Mat4RotateY(Mesh.rotation.y);
    mat4 ZRotationMatrix = Mat4RotateZ(Mesh.rotation.z);
    mat4 TranslateMatrix = Mat4Translate(Mesh.translation.x, Mesh.translation.y, Mesh.translation.z);

    Triangles = 0;
    for (i32 i = 0; i < array_length(Mesh.faces); ++i) {
      // Collect vertices of triangle for each face
      v3 FaceVerts[3];
      FaceVerts[0] = Mesh.vertices[Mesh.faces[i].a - 1];
      FaceVerts[1] = Mesh.vertices[Mesh.faces[i].b - 1];
      FaceVerts[2] = Mesh.vertices[Mesh.faces[i].c - 1];

      // Transform work
      for (u32 j = 0; j < arr_count(FaceVerts); ++j) {
        v4 NewVert = V3ToV4(FaceVerts[j]);
        // ORDER MATTERS!!!!! the below is really S*R*T
        // Builds like XRot*Scale -> YRot*(XRot*Scale) -> ZRot*(YRot*XRot*Scale) -> Translate*(ZRot*YRot*XRot*Scale)
        mat4 WorldMatrix = TranslateMatrix*ZRotationMatrix*YRotationMatrix*XRotationMatrix*ScaleMatrix;
        NewVert = Mat4MultV4(WorldMatrix, NewVert);
        FaceVerts[j] = V3FromV4(NewVert);
      }

        v3 FaceVertA = FaceVerts[0];
        v3 FaceVertB = FaceVerts[1];
        v3 FaceVertC = FaceVerts[2];
        v3 VectorBA = FaceVertB - FaceVertA;
        v3 VectorCA = FaceVertC - FaceVertA;
        V3Normalize(&VectorBA);
        V3Normalize(&VectorCA);
        v3 FaceNormal = CrossProduct(VectorBA, VectorCA);
        V3Normalize(&FaceNormal);

      // Backface Culling
      if (BackFaceCull) {
        v3 CameraRay = CameraPos - FaceVertA;
        // The DotProduct IS commutative!
        if (DotProduct(FaceNormal, CameraRay) <= 0) continue; // Skip projecting the vertices of this face as they are not visible
      }

      triangle CurrentTriangle = {0};
      // Projection work on each vertex of triangle
      for (u32 j = 0; j < arr_count(FaceVerts); ++j) {
        v4 ProjectedPoint = Mat4MultV4(ProjectionMatrix, V3ToV4(FaceVerts[j]));
        // Perspective divide
        if (ProjectedPoint.w != 0) {
          ProjectedPoint.x /= ProjectedPoint.w;
          ProjectedPoint.y /= ProjectedPoint.w;
          ProjectedPoint.z /= ProjectedPoint.w;
        }

        ProjectedPoint.x *= (f32)(WIN_WIDTH / 2);
        ProjectedPoint.y *= (f32)(WIN_HEIGHT / 2);

        ProjectedPoint.x += (f32)(WIN_WIDTH / 2);
        ProjectedPoint.y += (f32)(WIN_HEIGHT / 2);
        CurrentTriangle.vertices[j] = V2(ProjectedPoint.x, ProjectedPoint.y);
      }

      f32 AlignmentPercentage = -DotProduct(FaceNormal, GLight.direction);
      CurrentTriangle.color = ColorFromLightIntensity(Mesh.faces[i].color, AlignmentPercentage);
      CurrentTriangle.avg_depth = (FaceVerts[0].z + FaceVerts[1].z + FaceVerts[2].z) / 3.0f;
      array_push(Triangles, triangle, CurrentTriangle);
    }

    // NOTE: This is will not be efficient at all...
    // Will be very 'piggy' as i am copy swaps
    BubbleSortTrianlges(Triangles);

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
    if (RenderMode & FILLED) {
      for (i32 i = 0; i < array_length(Triangles); ++i) {
        DrawFilledTriangle(ColorBuff, (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y,
                           (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y,
                           (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y,
                           Triangles[i].color);
      }
    }
    if (RenderMode & WIREFRAME) {
      for (i32 i = 0; i < array_length(Triangles); ++i) {
        DrawLine(ColorBuff, (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, 0xFF00FF00);
        DrawLine(ColorBuff, (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, 0xFF00FF00);
        DrawLine(ColorBuff, (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, 0xFF00FF00);
      }
    }
    if (RenderMode & VERTICES) {
      for (i32 i = 0; i < array_length(Triangles); ++i) {
        DrawRect(ColorBuff, (u32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, 4, 4, 0xFFFF0000);
        DrawRect(ColorBuff, (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, 4, 4, 0xFFFF0000);
        DrawRect(ColorBuff, (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, 4, 4, 0xFFFF0000);
      }
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
