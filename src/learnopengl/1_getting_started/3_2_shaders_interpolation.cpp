#include "base/glfw_base.h"

#include <cmath>
#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common/shader.h"

class GlfwBaseCallbackImpl : public GlfwBaseCallback {
 public:
  GlfwBaseCallbackImpl() : shader_(nullptr) {
  }

  void OnGlfwInit(GlfwBase *) override {
    // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      return;
    }
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

    // Shader sources
    const char *vertexShaderSource = R"glsl(
      #version 330 core
      layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
      layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

      out vec3 ourColor; // output a color to the fragment shader

      void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor; // set ourColor to the input color we got from the vertex data
      })glsl";
    const char *fragmentShaderSource = R"glsl(
      #version 330 core
      out vec4 FragColor;
      in vec3 ourColor;

      void main() {
        FragColor = vec4(ourColor, 1.0);
      })glsl";

    // build and compile our shader program
    shader_ = std::unique_ptr<Shader>(new Shader(
      vertexShaderSource, fragmentShaderSource));

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
      // positions         // colors
       0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
       0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    shader_->Use();
  }

  void OnGlfwDraw(GlfwBase *glfw) override {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // render the triangle
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

 private:
  GLuint vao_;
  GLuint vbo_;

  std::unique_ptr<Shader> shader_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({400, 300, "GLFW Window"});
}
