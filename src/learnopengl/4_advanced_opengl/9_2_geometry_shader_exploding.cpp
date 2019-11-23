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

      out VS_OUT {
        vec2 texCoords;
      } vs_out;

      uniform mat4 projection;
      uniform mat4 view;
      uniform mat4 model;

      void main() {
        vs_out.texCoords = aTexCoords;
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
    )fs",
    R"gs(
      #version 330 core
      layout (triangles) in;
      layout (triangle_strip, max_vertices = 3) out;

      in VS_OUT {
        vec2 texCoords;
      } gs_in[];

      out vec2 TexCoords;

      uniform float time;

      vec4 explode(vec4 position, vec3 normal) {
        float magnitude = 2.0;
        vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
        return position + vec4(direction, 0.0);
      }

      vec3 GetNormal() {
        vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
        vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
        return normalize(cross(a, b));
      }

      void main() {
        vec3 normal = GetNormal();

        gl_Position = explode(gl_in[0].gl_Position, normal);
        TexCoords = gs_in[0].texCoords;
        EmitVertex();
        gl_Position = explode(gl_in[1].gl_Position, normal);
        TexCoords = gs_in[1].texCoords;
        EmitVertex();
        gl_Position = explode(gl_in[2].gl_Position, normal);
        TexCoords = gs_in[2].texCoords;
        EmitVertex();
        EndPrimitive();
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

    // add time component to geometry shader in the form of a uniform
    shader_.SetFloat("time", glfwGetTime());

    // draw model
    model_.Draw(shader_);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
  }

 private:
  Shader shader_;
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
