#pragma once

struct face_index {
  i32 a;
  i32 b;
  i32 c;
  u32 color;
};

struct triangle {
  v2 vertices[3];
  u32 color;
  f32 avg_depth;
};

struct mesh {
  v3 *vertices;
  face_index *faces;
  v3 scale;
  v3 rotation;
  v3 translation;
};

static mesh LoadMeshFromObjFile(char *FileName);

