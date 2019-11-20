#include "base/glfw_base.h"

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
      layout (location = 0) in vec2 aPos;
      layout (location = 1) in vec3 aColor;

      out VS_OUT {
        vec3 color;
      } vs_out;

      void main() {
        vs_out.color = aColor;
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
      })glsl";
    const char *fragmentShaderSource = R"glsl(
      #version 330 core
      out vec4 FragColor;

      in vec3 fColor;

      void main() {
        FragColor = vec4(fColor, 1.0);
      })glsl";
    const char *geometryShaderSource = R"glsl(
      #version 330 core
      layout (points) in;
      layout (triangle_strip, max_vertices = 5) out;

      in VS_OUT {
        vec3 color;
      } gs_in[];

      out vec3 fColor;

      void build_house(vec4 position) {
        fColor = gs_in[0].color; // gs_in[0] since there's only one input vertex
        gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); // 1:bottom-left
        EmitVertex();
        gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // 2:bottom-right
        EmitVertex();
        gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); // 3:top-left
        EmitVertex();
        gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); // 4:top-right
        EmitVertex();
        gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // 5:top
        fColor = vec3(1.0, 1.0, 1.0);
        EmitVertex();
        EndPrimitive();
      }

      void main() {
        build_house(gl_in[0].gl_Position);
      })glsl";

    // Load shader program
    shader_ = std::unique_ptr<Shader>(new Shader(
      vertexShaderSource, fragmentShaderSource, geometryShaderSource));

    // Set up vertex data: X, Y, R, G, B
    float vertices[] = {
      -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  // top-left
       0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  // top-right
       0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // bottom-right
      -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  // bottom-left
    };

    // Create Vertex Array Object
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Create Vertex Buffer Object
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
  }

  void OnGlfwDraw(GlfwBase *glfw) override {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw points
    shader_->Use();
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, 4);

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
