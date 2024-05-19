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

  Result.file_size = FileSize;
  Result.file_content = (char *)malloc(sizeof(char)*FileSize);
  while (fgets(Result.file_content, FileSize, FilePtr)) {
    switch (Result.file_content[0]) {
      case 'v': {
        // TODO: This is kinda hacky; probably should build str until space
        // and then do the switch on that str
        if (Result.file_content[1] != ' ') break;
        printf("%s\n", Result.file_content);
        v3 Vertex = {0};
        Result.file_content += 2;
        char VertComponent[16];
        char *VCPtr = VertComponent;
        char CurrentChar;
        u32 VertexIndex = 0;
        while ((CurrentChar = *Result.file_content++) != '\0') {
          if (CurrentChar == ' ' || CurrentChar == '\n') {
            Vertex.e[VertexIndex++] = (f32)atof(VertComponent);
            memset(VertComponent, 0, 16);
            VCPtr = &VertComponent[0];
          } else {
            *VCPtr++ = CurrentChar;
          }
        }
        array_push(Result.vertices, v3, Vertex);
      } break;
      case 'f': {
        printf("%s\n", Result.file_content);
        Result.file_content++;
        face_index Face = {0};
        char FaceIndexStr[8];
        char *FISPtr = FaceIndexStr;
        char CurrentChar;
        u32 FaceIndex = 0;
        while ((CurrentChar = *Result.file_content++) != '\0') {
          if (CurrentChar == ' ') {
            while ((CurrentChar = *Result.file_content++) != '/') {
              *FISPtr++ = CurrentChar;
            }
            Face.e[FaceIndex++] = atoi(FaceIndexStr);
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
