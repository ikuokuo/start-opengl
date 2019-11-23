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
    // glDepthMask(GL_TRUE);
    // glDepthFunc(GL_ALWAYS); // always pass the depth test (same effect as glDisable(GL_DEPTH_TEST))
    glDepthFunc(GL_LESS);

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

        // FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

        /*
        float depth = LinearizeDepth(gl_FragCoord.z) / far;  // divide by far for demonstration
        FragColor = vec4(vec3(depth), 1.0);
        */
      }

      /*
      float near = 0.1;
      float far  = 100.0;
      float LinearizeDepth(float depth);

      float LinearizeDepth(float depth) {
        float z = depth * 2.0 - 1.0; // back to NDC
        return (2.0 * near * far) / (far + near - z * (far - near));
      }
      */
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float cubeVertices[] = {
      // positions          // texture Coords
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    };
    float planeVertices[] = {
      // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
       5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
      -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
      -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

       5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
      -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
       5.0f, -0.5f, -5.0f,  2.0f, 2.0f,
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
    // plane VAO
    glGenVertexArrays(1, &plane_vao_);
    glGenBuffers(1, &plane_vbo_);
    glBindVertexArray(plane_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    cube_texture_ = LoadTexture(MY_DIR "/textures/marble.jpg");
    floor_texture_ = LoadTexture(MY_DIR "/textures/metal.png");

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
    // cubes
    glBindVertexArray(cube_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cube_texture_);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    shader_.SetMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    shader_.SetMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // floor
    glBindVertexArray(plane_vao_);
    glBindTexture(GL_TEXTURE_2D, floor_texture_);
    shader_.SetMat4("model", glm::mat4(1.0f));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &cube_vao_);
    glDeleteVertexArrays(1, &plane_vao_);
    glDeleteBuffers(1, &cube_vbo_);
    glDeleteBuffers(1, &plane_vbo_);
  }

 private:
  Shader shader_;
  GLuint cube_vao_;
  GLuint cube_vbo_;
  GLuint plane_vao_;
  GLuint plane_vbo_;
  GLuint cube_texture_;
  GLuint floor_texture_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
