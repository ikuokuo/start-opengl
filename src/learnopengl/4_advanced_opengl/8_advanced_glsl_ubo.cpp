#include "base/glfw_base.h"

#include <glm/gtc/type_ptr.hpp>

#include "common/camera.h"
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

    const char vertexShaderCode[] = R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;

      layout (std140) uniform Matrices {
        mat4 projection;
        mat4 view;
      };
      uniform mat4 model;

      void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs";

    shader_red_.Create(vertexShaderCode,
    R"fs(
      #version 330 core
      out vec4 FragColor;
      void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
      }
    )fs");
    shader_green_.Create(vertexShaderCode,
    R"fs(
      #version 330 core
      out vec4 FragColor;
      void main() {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
      }
    )fs");
    shader_blue_.Create(vertexShaderCode,
    R"fs(
      #version 330 core
      out vec4 FragColor;
      void main() {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
      }
    )fs");
    shader_yellow_.Create(vertexShaderCode,
    R"fs(
      #version 330 core
      out vec4 FragColor;
      void main() {
        FragColor = vec4(1.0, 1.0, 0.0, 1.0);
      }
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float cubeVertices[] = {
      // positions
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
    // cube VAO
    glGenVertexArrays(1, &cube_vao_);
    glGenBuffers(1, &cube_vbo_);
    glBindVertexArray(cube_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // configure a uniform buffer object
    // first. We get the relevant block indices
    GLuint uniformBlockIndexRed = glGetUniformBlockIndex(shader_red_.ID, "Matrices");
    GLuint uniformBlockIndexGreen = glGetUniformBlockIndex(shader_green_.ID, "Matrices");
    GLuint uniformBlockIndexBlue = glGetUniformBlockIndex(shader_blue_.ID, "Matrices");
    GLuint uniformBlockIndexYellow = glGetUniformBlockIndex(shader_yellow_.ID, "Matrices");
    // then we link each shader's uniform block to this uniform binding point
    glUniformBlockBinding(shader_red_.ID, uniformBlockIndexRed, 0);
    glUniformBlockBinding(shader_green_.ID, uniformBlockIndexGreen, 0);
    glUniformBlockBinding(shader_blue_.ID, uniformBlockIndexBlue, 0);
    glUniformBlockBinding(shader_yellow_.ID, uniformBlockIndexYellow, 0);
    // Now actually create the buffer
    glGenBuffers(1, &ubo_matrices_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices_, 0, 2 * sizeof(glm::mat4));

    // store the projection matrix (we only do this once now) (note: we're not using zoom anymore by changing the FoV)
    glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

    // set the view and projection matrix in the uniform block - we only have to do this once per loop iteration.
    glm::mat4 view = camera.GetViewMatrix();
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // draw 4 cubes
    // RED
    glBindVertexArray(cube_vao_);
    shader_red_.Use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f));  // move top-left
    shader_red_.SetMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // GREEN
    shader_green_.Use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f));  // move top-right
    shader_green_.SetMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // YELLOW
    shader_yellow_.Use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f));  // move bottom-left
    shader_yellow_.SetMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // BLUE
    shader_blue_.Use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f));  // move bottom-right
    shader_blue_.SetMat4("model", model);
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
  Shader shader_red_;
  Shader shader_green_;
  Shader shader_blue_;
  Shader shader_yellow_;

  GLuint cube_vao_;
  GLuint cube_vbo_;
  GLuint ubo_matrices_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
