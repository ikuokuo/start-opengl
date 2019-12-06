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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 1) in vec3 aNormal;
      layout (location = 2) in vec2 aTexCoords;

      // declare an interface block; see 'Advanced GLSL' for what these are.
      out VS_OUT {
        vec3 FragPos;
        vec3 Normal;
        vec2 TexCoords;
      } vs_out;

      uniform mat4 projection;
      uniform mat4 view;

      void main() {
        vs_out.FragPos = aPos;
        vs_out.Normal = aNormal;
        vs_out.TexCoords = aTexCoords;
        gl_Position = projection * view * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      in VS_OUT {
        vec3 FragPos;
        vec3 Normal;
        vec2 TexCoords;
      } fs_in;

      uniform sampler2D floorTexture;
      uniform vec3 lightPos;
      uniform vec3 viewPos;
      uniform bool blinn;

      void main() {
        vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
        // ambient
        vec3 ambient = 0.05 * color;
        // diffuse
        vec3 lightDir = normalize(lightPos - fs_in.FragPos);
        vec3 normal = normalize(fs_in.Normal);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * color;
        // specular
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        if(blinn) {
          vec3 halfwayDir = normalize(lightDir + viewDir);
          spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        } else {
          vec3 reflectDir = reflect(-lightDir, normal);
          spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
        }
        vec3 specular = vec3(0.3) * spec; // assuming bright white light color
        FragColor = vec4(ambient + diffuse + specular, 1.0);
      }
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float planeVertices[] = {
      // positions            // normals         // texcoords
       10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
      -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
      -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

       10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
      -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
       10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
    };
    // plane VAO
    glGenVertexArrays(1, &plane_vao_);
    glGenBuffers(1, &plane_vbo_);
    glBindVertexArray(plane_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    floor_texture_ = LoadTexture(MY_DIR "/textures/wood.png");

    // shader configuration
    shader_.Use();
    shader_.SetInt("texture1", 0);

    // lighting info
    light_pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

  void OnGlfwDraw(GlfwBase *glfw) override {
    static CameraHelper2 &camera = CameraHelper2::Instance();
    auto window = glfw->GetWindow();
    camera.OnFrame();
    camera.OnKeyEvent(window);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinn_key_pressed_)  {
      blinn_ = !blinn_;
      blinn_key_pressed_ = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)  {
      blinn_key_pressed_ = false;
    }

    // render
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    shader_.Use();
    glm::mat4 projection = camera.GetPerspectiveMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    shader_.SetMat4("projection", projection);
    shader_.SetMat4("view", view);
    // set light uniforms
    shader_.SetVec3("viewPos", camera.GetCamera().Position);
    shader_.SetVec3("lightPos", light_pos_);
    shader_.SetInt("blinn", blinn_);
    // floor
    glBindVertexArray(plane_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floor_texture_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    std::cout << (blinn_ ? "Blinn-Phong" : "Phong") << std::endl;

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &plane_vao_);
    glDeleteBuffers(1, &plane_vbo_);
  }

 private:
  Shader shader_;
  GLuint plane_vao_;
  GLuint plane_vbo_;
  GLuint floor_texture_;
  glm::vec3 light_pos_;
  bool blinn_ = false;
  bool blinn_key_pressed_ = false;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
