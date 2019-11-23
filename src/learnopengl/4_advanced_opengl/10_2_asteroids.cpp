#include "base/glfw_base.h"

#include <vector>

#include "common/camera.h"
#include "common/model.h"
#include "common/shader.h"

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

    shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 2) in vec2 aTexCoords;

      out vec2 TexCoords;

      uniform mat4 projection;
      uniform mat4 view;
      uniform mat4 model;

      void main() {
        TexCoords = aTexCoords;
        gl_Position = projection * view * model * vec4(aPos, 1.0f);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      in vec2 TexCoords;

      uniform sampler2D texture_diffuse1;

      void main() {
        FragColor = texture(texture_diffuse1, TexCoords);
      }
    )fs");

    rock_.Create(MY_DIR "/objects/rock/rock.obj");
    planet_.Create(MY_DIR "/objects/planet/planet.obj");

    rock_amount_ = 1000;
    rock_matrices_.reserve(rock_amount_);
    srand(glfwGetTime());  // initialize random seed
    float radius = 50.0;
    float offset = 2.5f;
    for (GLuint i = 0; i < rock_amount_; i++) {
      glm::mat4 model = glm::mat4(1.0f);
      // 1. translation: displace along circle with 'radius' in range [-offset, offset]
      float angle = (float)i / (float)rock_amount_ * 360.0f;
      float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
      float x = sin(angle) * radius + displacement;
      displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
      float y = displacement * 0.4f;  // keep height of asteroid field smaller compared to width of x and z
      displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
      float z = cos(angle) * radius + displacement;
      model = glm::translate(model, glm::vec3(x, y, z));

      // 2. scale: Scale between 0.05 and 0.25f
      float scale = (rand() % 20) / 100.0f + 0.05;
      model = glm::scale(model, glm::vec3(scale));

      // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
      float rotAngle = (rand() % 360);
      model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

      // 4. now add to list of matrices
      rock_matrices_[i] = model;
    }
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

    // configure transformation matrices
    glm::mat4 projection = camera.GetPerspectiveMatrix(0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader_.Use();
    shader_.SetMat4("projection", projection);
    shader_.SetMat4("view", view);

    // draw planet
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    shader_.SetMat4("model", model);
    planet_.Draw(shader_);

    // draw meteorites
    for (GLuint i = 0; i < rock_amount_; i++) {
      shader_.SetMat4("model", rock_matrices_[i]);
      rock_.Draw(shader_);
    }

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
  }

 private:
  Shader shader_;

  Model rock_;
  Model planet_;

  GLuint rock_amount_;
  std::vector<glm::mat4> rock_matrices_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 55.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
