#include "base/glfw_base.h"

#include "common/camera.h"
#include "common/shader.h"
#include "common/texture.h"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

class GlfwBaseCallbackImpl : public GlfwBaseCallback {
 public:
  GlfwBaseCallbackImpl() {
  }

  bool IsWindowCreatedOverride(GlfwBase *, GLFWwindow *) override { return true; }

  void OnGlfwInit(GlfwBase *glfw) override {
    CameraHelper2::glfw_init(glfw->GetWindow(), true);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 1) in vec2 aTexCoords;

      out vec2 TexCoords;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        TexCoords = aTexCoords;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      in vec2 TexCoords;

      uniform sampler2D texture1;

      void main() {
        FragColor = texture(texture1, TexCoords);
      }
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    /*
    Remember: to specify vertices in a counter-clockwise winding order you need to visualize the triangle
    as if you're in front of the triangle and from that point of view, is where you set their order.

    To define the order of a triangle on the right side of the cube for example, you'd imagine yourself looking
    straight at the right side of the cube, and then visualize the triangle and make sure their order is specified
    in a counter-clockwise order. This takes some practice, but try visualizing this yourself and see that this
    is correct.
    */
    float cubeVertices[] = {
      // Back face
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // top-right
       0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // bottom-right
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // top-right
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // bottom-left
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // top-left
      // Front face
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // bottom-left
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  // bottom-right
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // top-right
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // top-right
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  // top-left
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // bottom-left
      // Left face
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // top-right
      -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // top-left
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  // bottom-left
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  // bottom-left
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // bottom-right
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // top-right
      // Right face
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // top-left
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  // bottom-right
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // top-right
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  // bottom-right
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // top-left
       0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // bottom-left
      // Bottom face
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  // top-right
       0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  // top-left
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  // bottom-left
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  // bottom-left
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // bottom-right
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  // top-right
      // Top face
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // top-left
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // bottom-right
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // top-right
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // bottom-right
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // top-left
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  // bottom-left
    };
    // cube VAO
    glGenVertexArrays(1, &cube_vao_);
    glGenBuffers(1, &cube_vbo_);
    glBindVertexArray(cube_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    cube_texture_ = LoadTexture(MY_DIR "/textures/marble.jpg");

    // shader configuration
    shader_.Use();
    shader_.SetInt("texture1", 0);
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

    shader_.Use();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetPerspectiveMatrix();
    shader_.SetMat4("view", view);
    shader_.SetMat4("projection", projection);
    // cube
    glBindVertexArray(cube_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cube_texture_);
    shader_.SetMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &cube_vao_);
    glDeleteBuffers(1, &cube_vbo_);
  }

 private:
  Shader shader_;
  GLuint cube_vao_;
  GLuint cube_vbo_;
  GLuint cube_texture_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
