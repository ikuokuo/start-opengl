#include "base/glfw_base.h"

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
        gl_Position = projection * view * model * vec4(aPos, 1.0);
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
    normal_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 1) in vec3 aNormal;

      out VS_OUT {
        vec3 normal;
      } vs_out;

      uniform mat4 projection;
      uniform mat4 view;
      uniform mat4 model;

      void main() {
        mat3 normalMatrix = mat3(transpose(inverse(view * model)));
        vs_out.normal = vec3(projection * vec4(normalMatrix * aNormal, 0.0));
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      void main() {
        FragColor = vec4(1.0, 1.0, 0.0, 1.0);
      }
    )fs",
    R"gs(
      #version 330 core
      layout (triangles) in;
      layout (line_strip, max_vertices = 6) out;

      in VS_OUT {
        vec3 normal;
      } gs_in[];

      const float MAGNITUDE = 0.01;

      void GenerateLine(int index) {
        gl_Position = gl_in[index].gl_Position;
        EmitVertex();
        gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
        EmitVertex();
        EndPrimitive();
      }

      void main() {
        GenerateLine(0); // first vertex normal
        GenerateLine(1); // second vertex normal
        GenerateLine(2); // third vertex normal
      }
    )gs");

    model_.Create(MY_DIR "/objects/nanosuit/nanosuit.obj");
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
    glm::mat4 projection = camera.GetPerspectiveMatrix();
    glm::mat4 view = camera.GetViewMatrix();;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    shader_.Use();
    shader_.SetMat4("projection", projection);
    shader_.SetMat4("view", view);
    shader_.SetMat4("model", model);

    // draw model as usual
    model_.Draw(shader_);

    // then draw model with normal visualizing geometry shader
    normal_shader_.Use();
    normal_shader_.SetMat4("projection", projection);
    normal_shader_.SetMat4("view", view);
    normal_shader_.SetMat4("model", model);

    model_.Draw(normal_shader_);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
  }

 private:
  Shader shader_;
  Shader normal_shader_;
  Model model_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 4.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
