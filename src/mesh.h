#pragma once

union face_index {
  struct {
    i32 a;
    i32 b;
    i32 c;
  };
  i32 e[3];
};

struct triangle {
  v2 vertices[3];
};

struct mesh {
  v3 *vertices;
  face_index *faces;
  v3 rotation;
};

static mesh LoadMeshFromObjFile(char *FileName);

