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

    asteroid_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 2) in vec2 aTexCoords;
      layout (location = 3) in mat4 aInstanceMatrix;

      out vec2 TexCoords;

      uniform mat4 projection;
      uniform mat4 view;

      void main() {
        TexCoords = aTexCoords;
        gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0f);
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
    planet_shader_.Create(
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

    // generate a large list of semi-random model transformation matrices
    rock_amount_ = 100000;
    rock_matrices_.reserve(rock_amount_);
    srand(glfwGetTime());  // initialize random seed
    float radius = 150.0;
    float offset = 25.0f;
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

    // configure instanced array
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, rock_amount_ * sizeof(glm::mat4), &rock_matrices_[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    for (GLuint i = 0; i < rock_.meshes.size(); i++) {
      GLuint VAO = rock_.meshes[i].VAO;
      glBindVertexArray(VAO);
      // set attribute pointers for matrix (4 times vec4)
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
      glEnableVertexAttribArray(4);
      glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
      glEnableVertexAttribArray(5);
      glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
      glEnableVertexAttribArray(6);
      glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

      glVertexAttribDivisor(3, 1);
      glVertexAttribDivisor(4, 1);
      glVertexAttribDivisor(5, 1);
      glVertexAttribDivisor(6, 1);

      glBindVertexArray(0);
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
    asteroid_shader_.Use();
    asteroid_shader_.SetMat4("projection", projection);
    asteroid_shader_.SetMat4("view", view);
    planet_shader_.Use();
    planet_shader_.SetMat4("projection", projection);
    planet_shader_.SetMat4("view", view);

    // draw planet
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    planet_shader_.SetMat4("model", model);
    planet_.Draw(planet_shader_);

    // draw meteorites
    asteroid_shader_.Use();
    asteroid_shader_.SetInt("texture_diffuse1", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rock_.textures_loaded[0].id);  // note: we also made the textures_loaded vector public (instead of private) from the model class.
    for (GLuint i = 0; i < rock_.meshes.size(); i++) {
      glBindVertexArray(rock_.meshes[i].VAO);
      glDrawElementsInstanced(GL_TRIANGLES, rock_.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, rock_amount_);
      glBindVertexArray(0);
    }

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
  }

 private:
  Shader asteroid_shader_;
  Shader planet_shader_;

  Model rock_;
  Model planet_;

  GLuint rock_amount_;
  std::vector<glm::mat4> rock_matrices_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 155.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
