#pragma once

#include <GL/glew.h>

GLuint LoadShaderCodes(const char *vertex_shader_code,
                       const char *fragment_shader_code);
GLuint LoadShaderFiles(const char *vertex_file_path,
                       const char *fragment_file_path);
