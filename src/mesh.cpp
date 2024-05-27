#pragma once
#include "main.unity.h"
#include "mesh.h"

static mesh LoadMeshFromObjFile(char *FileName) {
  mesh Result = {0};
  FILE *FilePtr;
  neo_assert(fopen_s(&FilePtr, FileName, "r") == 0);
  neo_assert(FilePtr != 0);

  char FileLine[1024];
  while (fgets(FileLine, 1024, FilePtr)) {
    char *LinePtr = FileLine;
    switch (FileLine[0]) {
      case 'v': {
        // TODO: This is kinda hacky; probably should build str until space
        // and then do the switch on that str
        if (FileLine[1] != ' ') break;
        printf("%s\n", FileLine);
        v3 Vertex = {0};
        LinePtr += 2;
        char VertComponent[16];
        char *VCPtr = VertComponent;
        char CurrentChar;
        u32 VertexIndex = 0;
        while ((CurrentChar = *LinePtr++) != '\0') {
          if (CurrentChar == ' ' || CurrentChar == '\n') {
            if (VertexIndex == 1) {
              f32 Temp = (f32)atof(VertComponent);
              Temp *= -1.0f;
              Vertex.e[VertexIndex++] = Temp;
            } else {
              Vertex.e[VertexIndex++] = (f32)atof(VertComponent);
            }
            memset(VertComponent, 0, 16);
            VCPtr = &VertComponent[0];
          } else {
            *VCPtr++ = CurrentChar;
          }
        }
        array_push(Result.vertices, v3, Vertex);
      } break;
      case 'f': {
        printf("%s\n", FileLine);
        LinePtr++;
        face_index Face = {0};
        char FaceIndexStr[8];
        char *FISPtr = FaceIndexStr;
        char CurrentChar;
        u32 FaceIndex = 0;
        while ((CurrentChar = *LinePtr++) != '\0') {
          if (CurrentChar == ' ') {
            while ((CurrentChar = *LinePtr++) != '/') {
              *FISPtr++ = CurrentChar;
            }
            if (FaceIndex == 0) {
              Face.a = atoi(FaceIndexStr);
            } else if (FaceIndex == 1) {
              Face.b = atoi(FaceIndexStr);
            } else if (FaceIndex == 2) {
              Face.c = atoi(FaceIndexStr);
            }
            memset(FaceIndexStr, 0, 8);
            FISPtr = &FaceIndexStr[0];
          }
        }
        array_push(Result.faces, face_index, Face);
      } break;
      default: break;
    }
  }

  fclose(FilePtr);
  return Result;
}
