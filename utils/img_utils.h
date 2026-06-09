#pragma once
#include <windows.h>
#include <gl/GL.h>

GLuint LoadTexture(const char* filename, int* out_width = nullptr, int* out_height = nullptr);
