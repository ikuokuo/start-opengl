#pragma once

#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>

class Shader {
 public:
  GLuint ID;

  Shader() = default;
  Shader(const char *vertex_shader_code,
         const char *fragment_shader_code,
         const char *geometry_shader_code = nullptr) {
    Create(vertex_shader_code, fragment_shader_code, geometry_shader_code);
  }

  void Create(const char *vertex_shader_code,
              const char *fragment_shader_code,
              const char *geometry_shader_code = nullptr) {
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_code, NULL);
    glCompileShader(vertexShader);
    CheckCompileErrors(vertexShader, "VERTEX");

    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_code, NULL);
    glCompileShader(fragmentShader);
    CheckCompileErrors(fragmentShader, "FRAGMENT");

    // geometry shader
    GLuint geometryShader;
    if (geometry_shader_code != nullptr) {
      geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometryShader, 1, &geometry_shader_code, NULL);
      glCompileShader(geometryShader);
      CheckCompileErrors(geometryShader, "GEOMETRY");
    }

    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if (geometry_shader_code != nullptr)
      glAttachShader(shaderProgram, geometryShader);
    glLinkProgram(shaderProgram);
    CheckCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometry_shader_code != nullptr)
      glDeleteShader(geometryShader);

    ID = shaderProgram;
  }

  void Use() {
    glUseProgram(ID);
  }

  void SetBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }

  void SetInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }

  void SetFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }

  void SetVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void SetVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }

  void SetVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void SetVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }

  void SetVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void SetVec4(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
  }

  void SetMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

  void SetMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

  void SetMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

 private:
  void CheckCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if(!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if(!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }
  }
};
