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
        printf("%s\n", FileLine);
        FileLine[1] == 't' ? LinePtr += 3 : LinePtr += 2;
        v3 Vertex = {0};
        v2 UV = {0};
        char VertComponent[16];
        char *VCPtr = VertComponent;
        char CurrentChar;
        u32 Index = 0;
        while ((CurrentChar = *LinePtr++) != '\0') {
          if (CurrentChar == ' ' || CurrentChar == '\n') {
            if (FileLine[1] == 't') {
              UV.e[Index++] = (f32)atof(VertComponent);
            } else {
              Vertex.e[Index++] = (f32)atof(VertComponent);
            }
            memset(VertComponent, 0, 16);
            VCPtr = &VertComponent[0];
          } else {
            *VCPtr++ = CurrentChar;
          }
        }
        if (FileLine[1] == 't') {
          array_push(Result.uvs, v2, UV);
        } else {
          array_push(Result.vertices, v3, Vertex);
        }
      } break;
      case 'f': {
        printf("%s\n", FileLine);
        LinePtr += 2;
        face_index Face = {0};
        char FaceIndexStr[8];
        char *FISPtr = FaceIndexStr;
        char CurrentChar = *LinePtr;
        u32 FaceIndex = 0;
        u32 ValueCount = 0;
        while (*LinePtr != '\0') {
          if (CurrentChar == ' ') {
            ValueCount = 0;
            ++FaceIndex;
          }
          while ((CurrentChar = *LinePtr++) != '/') {
            if (CurrentChar == ' ' || CurrentChar == '\0') break;
            *FISPtr++ = CurrentChar;
          }
          ++ValueCount;
          if (ValueCount == 1) {
            if (FaceIndex == 0) {
              Face.a = atoi(FaceIndexStr);
            } else if (FaceIndex == 1) {
              Face.b = atoi(FaceIndexStr);
            } else if (FaceIndex == 2) {
              Face.c = atoi(FaceIndexStr);
            }
          } else if (ValueCount == 2) {
            if (FaceIndex == 0) {
              Face.a_uv = Result.uvs[atoi(FaceIndexStr) - 1];
            } else if (FaceIndex == 1) {
              Face.b_uv = Result.uvs[atoi(FaceIndexStr) - 1];
            } else if (FaceIndex == 2) {
              Face.c_uv = Result.uvs[atoi(FaceIndexStr) - 1];
            }
          }
          memset(FaceIndexStr, 0, 8);
          FISPtr = &FaceIndexStr[0];
        }
        Face.color = 0xFFFFFFFF;
        array_push(Result.faces, face_index, Face);
      } break;
      default: break;
    }
  }

  fclose(FilePtr);
  return Result;
}
