#pragma once
#include "main.unity.h"
#include "mesh.h"

static obj_file LoadObjFile(char *FileName) {
  obj_file Result = {0};
  FILE *FilePtr;
  neo_assert(fopen_s(&FilePtr, FileName, "r") == 0);
  neo_assert(FilePtr != 0);

  neo_assert(fseek(FilePtr, SEEK_SET, SEEK_END) == 0);
  u32 FileSize = ftell(FilePtr);
  neo_assert(fseek(FilePtr, SEEK_CUR, SEEK_SET) == 0);

  Result.FileSize = FileSize;
  Result.FileContent = (char *)malloc(sizeof(char)*FileSize);
  while (fgets(Result.FileContent, FileSize, FilePtr)) {
    switch (Result.FileContent[0]) {
      case 'v': {
        printf("%s\n", Result.FileContent);
        v3 Vertex = {0};
        Result.FileContent += 2;
        char VertComponent[16];
        char *VCPtr = VertComponent;
        char CurrentChar;
        u32 VertexIndex = 0;
        while ((CurrentChar = *Result.FileContent++) != '\0') {
          if (CurrentChar == ' ' || CurrentChar == '\n') {
            Vertex.e[VertexIndex++] = (f32)atof(VertComponent);
            memset(VertComponent, 0, 16);
            VCPtr = &VertComponent[0];
          } else {
            *VCPtr++ = CurrentChar;
          }
        }
        array_push(Result.VertexPositions, v3, Vertex);
      } break;
      case 'f': {
        printf("%s\n", Result.FileContent);
        Result.FileContent++;
        face_index Face = {0};
        char CurrentChar;
        u32 FaceIndex = 0;
        while ((CurrentChar = *Result.FileContent++) != '\0') {
          if (CurrentChar == ' ') {
            char VPValue = *Result.FileContent;
            Face.e[FaceIndex++] = atoi(&VPValue);
          }
        }
        array_push(Result.Faces, face_index, Face);
      } break;
      default: break;
    }
  }

  fclose(FilePtr);
  return Result;
}
