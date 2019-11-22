#include "base/glfw_base.h"

#include <iostream>

#include "common/camera.h"
#include "common/shader.h"

// settings
const unsigned int SCR_WIDTH = 400;
const unsigned int SCR_HEIGHT = 300;

class GlfwBaseCallbackImpl : public GlfwBaseCallback {
 public:
  GlfwBaseCallbackImpl() {
  }

  bool IsWindowCreatedOverride(GlfwBase *, GLFWwindow *) override { return true; }

  void OnGlfwInit(GlfwBase *glfw) override {
    CameraHelper2::glfw_init(glfw->GetWindow(), true);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    lighting_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      uniform vec3 objectColor;
      uniform vec3 lightColor;

      void main() {
        FragColor = vec4(lightColor * objectColor, 1.0);
      }
    )fs");

    lamp_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      void main() {
        FragColor = vec4(1.0); // set alle 4 vector values to 1.0
      }
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
      -0.5f, -0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       0.5f,  0.5f, -0.5f,
       0.5f,  0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,

      -0.5f, -0.5f,  0.5f,
       0.5f, -0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f, -0.5f,  0.5f,

      -0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,

       0.5f,  0.5f,  0.5f,
       0.5f,  0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       0.5f, -0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,

      -0.5f, -0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       0.5f, -0.5f,  0.5f,
       0.5f, -0.5f,  0.5f,
      -0.5f, -0.5f,  0.5f,
      -0.5f, -0.5f, -0.5f,

      -0.5f,  0.5f, -0.5f,
       0.5f,  0.5f, -0.5f,
       0.5f,  0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f, -0.5f,
    };

    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &cube_vao_);
    glGenBuffers(1, &vbo_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cube_vao_);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    glGenVertexArrays(1, &light_vao_);
    glBindVertexArray(light_vao_);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

  void OnGlfwDraw(GlfwBase *glfw) override {
    static CameraHelper2 &camera = CameraHelper2::Instance();
    // per-frame
    camera.OnFrame();
    // input
    camera.OnKeyEvent(glfw->GetWindow());

    // render
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    lighting_shader_.Use();
    lighting_shader_.SetVec3("objectColor", 1.0f, 0.5f, 0.31f);
    lighting_shader_.SetVec3("lightColor",  1.0f, 1.0f, 1.0f);

    // view/projection transformations
    glm::mat4 projection = camera.GetPerspectiveMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    lighting_shader_.SetMat4("projection", projection);
    lighting_shader_.SetMat4("view", view);

    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    lighting_shader_.SetMat4("model", model);

    // render the cube
    glBindVertexArray(cube_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    // also draw the lamp object
    lamp_shader_.Use();
    lamp_shader_.SetMat4("projection", projection);
    lamp_shader_.SetMat4("view", view);
    model = glm::mat4(1.0f);
    model = glm::translate(model, light_pos_);
    model = glm::scale(model, glm::vec3(0.2f));  // a smaller cube
    lamp_shader_.SetMat4("model", model);

    glBindVertexArray(light_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &cube_vao_);
    glDeleteVertexArrays(1, &light_vao_);
    glDeleteBuffers(1, &vbo_);
  }

 private:
  Shader lighting_shader_;
  Shader lamp_shader_;
  GLuint cube_vao_;
  GLuint light_vao_;
  GLuint vbo_;
  // lighting
  glm::vec3 light_pos_{1.2f, 1.0f, 2.0f};
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 5.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
