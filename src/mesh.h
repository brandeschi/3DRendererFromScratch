#pragma once

struct face_index {
  i32 a;
  i32 b;
  i32 c;
  v2 a_uv;
  v2 b_uv;
  v2 c_uv;
  u32 color;
};

struct triangle {
  v4 vertices[3];
  v2 texture_coords[3];
  u32 color;
  f32 avg_depth;
};

struct mesh {
  v3 *vertices;
  face_index *faces;
  v2 *uvs;
  v3 scale;
  v3 rotation;
  v3 translation;
};

static mesh LoadMeshFromObjFile(char *FileName);

