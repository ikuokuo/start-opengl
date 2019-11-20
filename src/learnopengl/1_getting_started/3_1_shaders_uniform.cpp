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
    glewExperimental = true;  // Needed in core profile
    if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      return;
    }
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

    // Shader sources
    const char *vertexShaderSource = R"glsl(
      #version 330 core
      layout (location = 0) in vec3 aPos;

      void main() {
        gl_Position = vec4(aPos, 1.0);
      })glsl";
    const char *fragmentShaderSource = R"glsl(
      #version 330 core
      out vec4 FragColor;
      uniform vec4 ourColor;

      void main() {
        FragColor = ourColor;
      })glsl";

    // build and compile our shader program
    shader_ = std::unique_ptr<Shader>(new Shader(
      vertexShaderSource, fragmentShaderSource));

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
       0.5f, -0.5f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f,  // bottom left
       0.0f,  0.5f, 0.0f,  // top
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    // bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can
    // just bind it beforehand before rendering the respective triangle; this is another approach.
    glBindVertexArray(vao_);
  }

  void OnGlfwDraw(GlfwBase *glfw) override {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // be sure to activate the shader before any calls to glUniform
    shader_->Use();

    // update shader uniform
    float timeValue = glfwGetTime();
    float greenValue = sin(timeValue) / 2.0f + 0.5f;
    // int vertexColorLocation = glGetUniformLocation(shader_->ID, "ourColor");
    // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
    shader_->SetVec4("ourColor", 0.0f, greenValue, 0.0f, 1.0f);

    // render the triangle
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
