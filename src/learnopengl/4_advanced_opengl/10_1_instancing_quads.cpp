#include "base/glfw_base.h"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common/shader.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

class GlfwBaseCallbackImpl : public GlfwBaseCallback {
 public:
  GlfwBaseCallbackImpl() {
  }

  void OnGlfwInit(GlfwBase *) override {
    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      return;
    }
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec2 aPos;
      layout (location = 1) in vec3 aColor;
      layout (location = 2) in vec2 aOffset;

      out vec3 fColor;

      void main() {
        fColor = aColor;
        // gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
        vec2 pos = aPos * (gl_InstanceID / 100.0);
        gl_Position = vec4(pos + aOffset, 0.0, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      in vec3 fColor;

      void main() {
        FragColor = vec4(fColor, 1.0);
      }
    )fs");

    // generate a list of 100 quad locations/translation-vectors
    glm::vec2 translations[100];
    int index = 0;
    float offset = 0.1f;
    for (int y = -10; y < 10; y += 2) {
      for (int x = -10; x < 10; x += 2) {
        glm::vec2 translation;
        translation.x = (float)x / 10.0f + offset;
        translation.y = (float)y / 10.0f + offset;
        translations[index++] = translation;
      }
    }

    // store instance data in an array buffer
    glGenBuffers(1, &instance_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float quadVertices[] = {
      // positions     // colors
      -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
       0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
      -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

      -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
       0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
       0.05f,  0.05f,  0.0f, 1.0f, 1.0f,
    };
    glGenVertexArrays(1, &quad_vao_);
    glGenBuffers(1, &quad_vbo_);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    // also set instance data
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);  // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);  // tell OpenGL this is an instanced vertex attribute.
  }

  void OnGlfwDraw(GlfwBase *glfw) override {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw 100 instanced quads
    shader_.Use();
    glBindVertexArray(quad_vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);  // 100 triangles of 6 vertices each
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &quad_vao_);
    glDeleteBuffers(1, &quad_vbo_);
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

 private:
  GLuint instance_vbo_;
  GLuint quad_vao_;
  GLuint quad_vbo_;
  Shader shader_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
