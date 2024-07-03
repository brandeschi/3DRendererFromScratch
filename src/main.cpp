#include "main.unity.h"

f32 *DepthBuff;

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

v3 BarycentricWeights(v2 a, v2 b, v2 c, v2 p) {
  v2 AB = b - a;
  v2 AC = c - a;
  v2 PC = c - p;
  v2 PB = b - p;
  v2 AP = p - a;

  f32 ParallelAreaABC = (AC.x*AB.y) - (AC.y*AB.x); // Length of the imaginary 'Z' component of the two 2-D vectors
  f32 Alpha = ((PC.x*PB.y) - (PC.y*PB.x)) / ParallelAreaABC;
  f32 Beta = ((AC.x*AP.y) - (AC.y*AP.x)) / ParallelAreaABC;
  f32 Gamma = 1.0f - Alpha - Beta;

  v3 Weights = V3(Alpha, Beta, Gamma);
  return Weights;
}

triangle *Triangles = 0;
global mesh Mesh = {0};

// Per bit option for rendering mode
// TODO: Make this a composiable system using XOR?
enum RenderModeEnum {
  WIREFRAME = 1 << 0,
  FILLED = 1 << 1,
  VERTICES = 1 << 2,
  TEXTURED = 1 << 3,
};

global b32 AppRunning = false;
global u32 PrevFrameTime = 0;
global f32 DeltaTime = 0.0f;
global i32 SyncTime = 0;
global b32 BackFaceCull = true;
global u32 RenderMode = FILLED;
#define FPS 60
#define TARGET_FRAME_TIME (1000 / FPS)
global const u32 WIN_WIDTH = 1200;
global const u32 WIN_HEIGHT = 900;
global const f32 FOV_FACTOR = 640.0f;

inline void SwapI32(i32 *a, i32 *b) {
  i32 temp = *a;
  *a = *b;
  *b = temp;
}
inline void SwapF32(f32 *a, f32 *b) {
  f32 temp = *a;
  *a = *b;
  *b = temp;
}
inline void SwapTriangles(triangle *a, triangle *b) {
  triangle temp = *a;
  *a = *b;
  *b = temp;
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

static void DrawFilledTriangle(u32 *ColorBuffer,
                               i32 x0, i32 y0, f32 z0, f32 w0,
                               i32 x1, i32 y1, f32 z1, f32 w1,
                               i32 x2, i32 y2, f32 z2, f32 w2,
                               u32 Color) {
  // NOTE: This uses the Flat-top/Flat-bottom method
  // Ensure y's are sorted with y0 being the smallest and y2 being the largest
  if (y0 > y1) {
    SwapI32(&y0, &y1);
    SwapI32(&x0, &x1);
    SwapF32(&z0, &z1);
    SwapF32(&w0, &w1);
  }
  if (y1 > y2) {
    SwapI32(&y1, &y2);
    SwapI32(&x1, &x2);
    SwapF32(&z1, &z2);
    SwapF32(&w1, &w2);
  }
  if (y0 > y1) {
    SwapI32(&y0, &y1);
    SwapI32(&x0, &x1);
    SwapF32(&z0, &z1);
    SwapF32(&w0, &w1);
  }

  v4 VertexA = V4((f32)x0, (f32)y0, z0, w0);
  v4 VertexB = V4((f32)x1, (f32)y1, z1, w1);
  v4 VertexC = V4((f32)x2, (f32)y2, z2, w2);

  // Flat-Bottom
  f32 InvSlope1 = 0.0f;
  f32 InvSlope2 = 0.0f;

  if (y1 - y0 != 0) InvSlope1 = (f32)(x1 - x0) / (f32)abs(y1 - y0);
  if (y2 - y0 != 0) InvSlope2 = (f32)(x2 - x0) / (f32)abs(y2 - y0);

  if (y1 - y0 != 0) {
    for (i32 Row = y0; Row <= y1; ++Row) {
      i32 StartX = (i32)(x1 + (Row - y1)*InvSlope1);
      i32 EndX = (i32)(x0 + (Row - y0)*InvSlope2);
      if (EndX < StartX) {
        SwapI32(&StartX, &EndX);
      }

      for (i32 Col = (i32)StartX; Col < (i32)EndX; ++Col) {
        v3 Weights = BarycentricWeights(V2(VertexA.x, VertexA.y),
                                        V2(VertexB.x, VertexB.y),
                                        V2(VertexC.x, VertexC.y),
                                        V2((f32)Col, (f32)Row));
        f32 InterpolatedReciprocalW = (1.0f / VertexA.w)*Weights.x +
                                      (1.0f / VertexB.w)*Weights.y +
                                      (1.0f / VertexC.w)*Weights.z;
        InterpolatedReciprocalW = 1.0f - InterpolatedReciprocalW;
        if (InterpolatedReciprocalW < DepthBuff[(WIN_WIDTH*Row) + Col]) {
          ColorBuffer[(WIN_WIDTH*Row) + Col] = Color;
          DepthBuff[(WIN_WIDTH*Row) + Col] = InterpolatedReciprocalW;
        }
      }
    }
  }

  // Flat-Top
  InvSlope1 = 0.0f;
  InvSlope2 = 0.0f;

  if (y2 - y1 != 0) InvSlope1 = (f32)(x2 - x1) / (f32)abs(y2 - y1);
  if (y2 - y0 != 0) InvSlope2 = (f32)(x2 - x0) / (f32)abs(y2 - y0);

  if (y2 - y1 != 0) {
    for (i32 Row = y1; Row <= y2; ++Row) {
      i32 StartX = (i32)(x1 + (Row - y1)*InvSlope1);
      i32 EndX = (i32)(x0 + (Row - y0)*InvSlope2);
      if (EndX < StartX) {
        SwapI32(&StartX, &EndX);
      }

      for (i32 Col = (i32)StartX; Col < (i32)EndX; ++Col) {
        v3 Weights = BarycentricWeights(V2(VertexA.x, VertexA.y),
                                        V2(VertexB.x, VertexB.y),
                                        V2(VertexC.x, VertexC.y),
                                        V2((f32)Col, (f32)Row));
        f32 InterpolatedReciprocalW = (1.0f / VertexA.w)*Weights.x +
                                      (1.0f / VertexB.w)*Weights.y +
                                      (1.0f / VertexC.w)*Weights.z;
        InterpolatedReciprocalW = 1.0f - InterpolatedReciprocalW;
        if (InterpolatedReciprocalW < DepthBuff[(WIN_WIDTH*Row) + Col]) {
          ColorBuffer[(WIN_WIDTH*Row) + Col] = Color;
          DepthBuff[(WIN_WIDTH*Row) + Col] = InterpolatedReciprocalW;
        }
      }
    }
  }
}

static void DrawTexel(u32 *ColorBuffer,
                      i32 x, i32 y,
                      v4 VertexA, v4 VertexB, v4 VertexC,
                      u32 *Texture, v2 *uvs) {
  v3 Weights = BarycentricWeights(V2(VertexA.x, VertexA.y),
                                  V2(VertexB.x, VertexB.y),
                                  V2(VertexC.x, VertexC.y),
                                  V2((f32)x, (f32)y));

  f32 InterpolatedU = ((uvs[0].u / VertexA.w)*Weights.x) +
                       ((uvs[1].u / VertexB.w)*Weights.y) +
                       ((uvs[2].u / VertexC.w)*Weights.z);
  f32 InterpolatedV = ((uvs[0].v / VertexA.w)*Weights.x) +
                       ((uvs[1].v / VertexB.w)*Weights.y) +
                       ((uvs[2].v / VertexC.w)*Weights.z);
  f32 InterpolatedReciprocalW = (1.0f / VertexA.w)*Weights.x +
                                (1.0f / VertexB.w)*Weights.y +
                                (1.0f / VertexC.w)*Weights.z;
  InterpolatedU /= InterpolatedReciprocalW;
  InterpolatedV /= InterpolatedReciprocalW;

  i32 TextureX = abs((i32)(InterpolatedU*TextureWidth));
  i32 TextureY = abs((i32)(InterpolatedV*TextureHeight));

  InterpolatedReciprocalW = 1.0f - InterpolatedReciprocalW;
  if (InterpolatedReciprocalW < DepthBuff[(WIN_WIDTH*y) + x]) {
    i32 TextureIndex = (((TextureWidth*TextureY) + TextureX) % (TextureWidth*TextureHeight));
    ColorBuffer[(WIN_WIDTH*y) + x] = Texture[TextureIndex];
    DepthBuff[(WIN_WIDTH*y) + x] = InterpolatedReciprocalW;
  }
}

static void DrawTexturedTriangle(u32 *ColorBuffer,
                                 i32 x0, i32 y0, f32 z0, f32 w0,
                                 i32 x1, i32 y1, f32 z1, f32 w1,
                                 i32 x2, i32 y2, f32 z2, f32 w2,
                                 u32 *texture, v2 *uvs) {
  // NOTE: This uses the Flat-top/Flat-bottom method
  // Ensure y's are sorted with y0 being the smallest and y2 being the largest
  if (y0 > y1) {
    SwapI32(&x0, &x1);
    SwapI32(&y0, &y1);
    SwapF32(&z0, &z1);
    SwapF32(&w0, &w1);
    SwapF32(&uvs[0].v, &uvs[1].v);
    SwapF32(&uvs[0].u, &uvs[1].u);
  }
  if (y1 > y2) {
    SwapI32(&x1, &x2);
    SwapI32(&y1, &y2);
    SwapF32(&z1, &z2);
    SwapF32(&w1, &w2);
    SwapF32(&uvs[1].v, &uvs[2].v);
    SwapF32(&uvs[1].u, &uvs[2].u);
  }
  if (y0 > y1) {
    SwapI32(&x0, &x1);
    SwapI32(&y0, &y1);
    SwapF32(&z0, &z1);
    SwapF32(&w0, &w1);
    SwapF32(&uvs[0].v, &uvs[1].v);
    SwapF32(&uvs[0].u, &uvs[1].u);
  }

  // Flipping the V coord of the texture as they go from bottom left instead of top left
  // uvs[0].u = 1.0f - uvs[0].u;
  // uvs[1].u = 1.0f - uvs[1].u;
  // uvs[2].u = 1.0f - uvs[2].u;
  uvs[0].v = 1.0f - uvs[0].v;
  uvs[1].v = 1.0f - uvs[1].v;
  uvs[2].v = 1.0f - uvs[2].v;

  v4 VertexA = V4((f32)x0, (f32)y0, z0, w0);
  v4 VertexB = V4((f32)x1, (f32)y1, z1, w1);
  v4 VertexC = V4((f32)x2, (f32)y2, z2, w2);

  // Flat-Bottom
  f32 InvSlope1 = 0.0f;
  f32 InvSlope2 = 0.0f;

  if (y1 - y0 != 0) InvSlope1 = (f32)(x1 - x0) / (f32)abs(y1 - y0);
  if (y2 - y0 != 0) InvSlope2 = (f32)(x2 - x0) / (f32)abs(y2 - y0);

  if (y1 - y0 != 0) {
    for (i32 Row = y0; Row <= y1; ++Row) {
      i32 StartX = (i32)(x1 + (Row - y1)*InvSlope1);
      i32 EndX = (i32)(x0 + (Row - y0)*InvSlope2);
      if (EndX < StartX) {
        SwapI32(&StartX, &EndX);
      }

      for (i32 Col = (i32)StartX; Col < (i32)EndX; ++Col) {
        DrawTexel(ColorBuffer, Col, Row, VertexA, VertexB, VertexC, texture, uvs);
      }
    }
  }

  // Flat-Top
  InvSlope1 = 0.0f;
  InvSlope2 = 0.0f;

  if (y2 - y1 != 0) InvSlope1 = (f32)(x2 - x1) / (f32)abs(y2 - y1);
  if (y2 - y0 != 0) InvSlope2 = (f32)(x2 - x0) / (f32)abs(y2 - y0);

  if (y2 - y1 != 0) {
    for (i32 Row = y1; Row <= y2; ++Row) {
      i32 StartX = (i32)(x1 + (Row - y1)*InvSlope1);
      i32 EndX = (i32)(x0 + (Row - y0)*InvSlope2);
      if (EndX < StartX) {
        SwapI32(&StartX, &EndX);
      }

      for (i32 Col = (i32)StartX; Col < (i32)EndX; ++Col) {
        DrawTexel(ColorBuffer, Col, Row, VertexA, VertexB, VertexC, texture, uvs);
      }
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
  DepthBuff = (f32 *)malloc(sizeof(f32)*WIN_WIDTH*WIN_HEIGHT);
  if (!ColorBuff) {
    fprintf(stderr, "Error allocating with malloc\n");
    return -1;
  }
  SDL_Texture *CBTexture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH, WIN_HEIGHT);

  // mesh CubeMesh = LoadMeshFromObjFile("./assets/cube.obj");
  // upng_t *CubeTexture = LoadPNGTextureFromFile("./assets/cube.png");
  // mesh SphereMesh = LoadMeshFromObjFile("./assets/sphere.obj");
  // upng_t *SphereTexture = LoadPNGTextureFromFile("./assets/sphere.png");
  // mesh F22Mesh = LoadMeshFromObjFile("./assets/f22.obj");
  // upng_t *F22Texture = LoadPNGTextureFromFile("./assets/f22.png");
  mesh F117Mesh = LoadMeshFromObjFile("./assets/f117.obj");
  upng_t *F117Texture = LoadPNGTextureFromFile("./assets/f117.png");
  // mesh CrabMesh = LoadMeshFromObjFile("./assets/crab.obj");
  // upng_t *CrabTexture = LoadPNGTextureFromFile("./assets/crab.png");
  // mesh DroneMesh = LoadMeshFromObjFile("./assets/drone.obj");
  // upng_t *DroneTexture = LoadPNGTextureFromFile("./assets/drone.png");
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
  { 1, 2, 3, { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, 0xFFFFFFFF },
  { 1, 3, 4, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, 0xFFFFFFFF },
  // right
  { 4, 3, 5, { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, 0xFFFFFFFF },
  { 4, 5, 6, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, 0xFFFFFFFF },
  // back
  { 6, 5, 7, { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, 0xFFFFFFFF },
  { 6, 7, 8, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, 0xFFFFFFFF },
  // left
  { 8, 7, 2, { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, 0xFFFFFFFF },
  { 8, 2, 1, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, 0xFFFFFFFF },
  // top
  { 2, 7, 5, { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, 0xFFFFFFFF },
  { 2, 5, 3, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, 0xFFFFFFFF },
  // bottom
  { 6, 8, 1, { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, 0xFFFFFFFF },
  { 6, 1, 4, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, 0xFFFFFFFF }
};

  // Load cube mesh data
  for (u32 i = 0; i < CUBE_VERTICES_COUNT; ++i) {
    array_push(Mesh.vertices, v3, CubeVertices[i]);
  }
  for (u32 i = 0; i < CUBE_FACE_COUNT; ++i) {
    array_push(Mesh.faces, face_index, CubeFaces[i]);
  }

  // Mesh.vertices = CubeMesh.vertices;
  // Mesh.faces = CubeMesh.faces;
  // Mesh.vertices = SphereMesh.vertices;
  // Mesh.faces = SphereMesh.faces;
  // Mesh.vertices = F22Mesh.vertices;
  // Mesh.faces = F22Mesh.faces;
  Mesh.vertices = F117Mesh.vertices;
  Mesh.faces = F117Mesh.faces;
  // Mesh.vertices = CrabMesh.vertices;
  // Mesh.faces = CrabMesh.faces;
  // Mesh.vertices = DroneMesh.vertices;
  // Mesh.faces = DroneMesh.faces;

  camera Camera = {
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
  };
  f32 FOV = PI32 / 3.0f;
  f32 ZNear = 0.1f;
  f32 ZFar = 100.0f;
  mat4 ProjectionMatrix = Mat4Projection(((f32)WIN_HEIGHT / (f32)WIN_WIDTH), FOV, ZNear, ZFar);
  light GLight = { 0.0f, 0.0f, 1.0f };

  AppRunning = true;
  while (AppRunning) {
    // INPUT
    SDL_Event Event;
    while(SDL_PollEvent(&Event)) {
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
          if(Event.key.keysym.sym == SDLK_5) {
            RenderMode = TEXTURED;
          }
          if(Event.key.keysym.sym == SDLK_6) {
            RenderMode = WIREFRAME | TEXTURED;
          }
          if(Event.key.keysym.sym == SDLK_c) {
            BackFaceCull = true;
          }
          if(Event.key.keysym.sym == SDLK_d) {
            BackFaceCull = false;
          }
        } break;
      }
    }


    // UPDATE

    // Mesh.scale.x += 0.002f;

    Mesh.rotation.x += 0.5f*DeltaTime;
    // Mesh.rotation.y += 5.0f*DeltaTime;
    // Mesh.rotation.z += 5.0f*DeltaTime;

    // Mesh.translation.x += 5.0f*DeltaTime;
    Mesh.translation.z = 5.0f;

    Camera.position.x += 0.3f*DeltaTime;
    Camera.position.y += 0.3f*DeltaTime;

    v3 Target = Mesh.translation;
    mat4 ViewMatrix = M4LookAt(Camera.position, Target, { 0.0f, 1.0f, 0.0f });

    mat4 ScaleMatrix = Mat4Scale(Mesh.scale.x, Mesh.scale.y, Mesh.scale.z);
    mat4 XRotationMatrix = Mat4RotateX(Mesh.rotation.x);
    mat4 YRotationMatrix = Mat4RotateY(Mesh.rotation.y);
    mat4 ZRotationMatrix = Mat4RotateZ(Mesh.rotation.z);
    mat4 TranslationMatrix = Mat4Translate(Mesh.translation.x, Mesh.translation.y, Mesh.translation.z);

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
        mat4 WorldMatrix = TranslationMatrix*ZRotationMatrix*YRotationMatrix*XRotationMatrix*ScaleMatrix;
        NewVert = Mat4MultV4(WorldMatrix, NewVert);
        NewVert = Mat4MultV4(ViewMatrix, NewVert);
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
        v3 CameraRay = V3(0.0f, 0.0f, 0.0f) - FaceVertA;
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

        ProjectedPoint.y *= -1.0f;

        ProjectedPoint.x += (f32)(WIN_WIDTH / 2);
        ProjectedPoint.y += (f32)(WIN_HEIGHT / 2);
        CurrentTriangle.vertices[j] = V4(ProjectedPoint.x, ProjectedPoint.y, ProjectedPoint.z, ProjectedPoint.w);
      }

      CurrentTriangle.texture_coords[0] = { Mesh.faces[i].a_uv.u, Mesh.faces[i].a_uv.v };
      CurrentTriangle.texture_coords[1] = { Mesh.faces[i].b_uv.u, Mesh.faces[i].b_uv.v };
      CurrentTriangle.texture_coords[2] = { Mesh.faces[i].c_uv.u, Mesh.faces[i].c_uv.v };
      f32 AlignmentPercentage = -DotProduct(FaceNormal, GLight.direction);
      CurrentTriangle.color = ColorFromLightIntensity(Mesh.faces[i].color, AlignmentPercentage);
      array_push(Triangles, triangle, CurrentTriangle);
    }

    // RENDER

    // Clear
    for (u32 y = 0; y < WIN_HEIGHT; ++y) {
      for (u32 x = 0; x < WIN_WIDTH; ++x) {
        // Darker Background
        ColorBuff[(WIN_WIDTH * y) + x] = 0xFF3A4253;
        // Start all depths at the zFar
        DepthBuff[(WIN_WIDTH * y) + x] = 1.0f;
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
        DrawFilledTriangle(ColorBuff,
                           (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, Triangles[i].vertices[0].z, Triangles[i].vertices[0].w,
                           (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, Triangles[i].vertices[1].z, Triangles[i].vertices[1].w,
                           (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, Triangles[i].vertices[2].z, Triangles[i].vertices[2].w,
                           Triangles[i].color);
      }
    }
    if (RenderMode & TEXTURED) {
      for (i32 i = 0; i < array_length(Triangles); ++i) {
        DrawTexturedTriangle(ColorBuff,
                             (i32)Triangles[i].vertices[0].x, (i32)Triangles[i].vertices[0].y, Triangles[i].vertices[0].z, Triangles[i].vertices[0].w,
                             (i32)Triangles[i].vertices[1].x, (i32)Triangles[i].vertices[1].y, Triangles[i].vertices[1].z, Triangles[i].vertices[1].w,
                             (i32)Triangles[i].vertices[2].x, (i32)Triangles[i].vertices[2].y, Triangles[i].vertices[2].z, Triangles[i].vertices[2].w,
                             MeshTexture,
                             Triangles[i].texture_coords);
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

    // Time between frames in seconds
    DeltaTime = (f32)((SDL_GetTicks() - PrevFrameTime) / 1000.0f);

    PrevFrameTime = SDL_GetTicks();
  }

  return 0;
}
