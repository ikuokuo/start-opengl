#include "base/glfw_base.h"

#include <vector>

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

      uniform vec3 lightPositions[4];
      uniform vec3 lightColors[4];
      uniform vec3 viewPos;
      uniform bool gamma;

      vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor) {
        // diffuse
        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * lightColor;
        // specular
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lightColor;
        // simple attenuation
        float max_distance = 1.5;
        float distance = length(lightPos - fragPos);
        float attenuation = 1.0 / (gamma ? distance * distance : distance);

        diffuse *= attenuation;
        specular *= attenuation;

        return diffuse + specular;
      }

      void main() {
        vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
        vec3 lighting = vec3(0.0);
        for(int i = 0; i < 4; ++i)
          lighting += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPositions[i], lightColors[i]);
        color *= lighting;
        if(gamma)
          color = pow(color, vec3(1.0/2.2));
        FragColor = vec4(color, 1.0);
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
    floor_texture_                 = LoadTexture(MY_DIR "/textures/wood.png", false);
    floor_texture_gamma_corrected_ = LoadTexture(MY_DIR "/textures/wood.png", true);

    // shader configuration
    shader_.Use();
    shader_.SetInt("floorTexture", 0);

    // lighting info
    light_positions_ = {
      glm::vec3(-3.0f, 0.0f, 0.0f),
      glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3 (1.0f, 0.0f, 0.0f),
      glm::vec3 (3.0f, 0.0f, 0.0f),
    };
    light_colors_ = {
      glm::vec3(0.25),
      glm::vec3(0.50),
      glm::vec3(0.75),
      glm::vec3(1.00),
    };
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

  void OnGlfwDraw(GlfwBase *glfw) override {
    static CameraHelper2 &camera = CameraHelper2::Instance();
    auto window = glfw->GetWindow();
    camera.OnFrame();
    camera.OnKeyEvent(window);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !gamma_key_pressed_)  {
      gamma_enabled_ = !gamma_enabled_;
      gamma_key_pressed_ = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)  {
      gamma_key_pressed_ = false;
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
    glUniform3fv(glGetUniformLocation(shader_.ID, "lightPositions"), 4, &light_positions_[0][0]);
    glUniform3fv(glGetUniformLocation(shader_.ID, "lightColors"), 4, &light_colors_[0][0]);
    shader_.SetVec3("viewPos", camera.GetCamera().Position);
    shader_.SetInt("gamma", gamma_enabled_);
    // floor
    glBindVertexArray(plane_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gamma_enabled_ ? floor_texture_gamma_corrected_ : floor_texture_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    std::cout << (gamma_enabled_ ? "Gamma enabled" : "Gamma disabled") << std::endl;

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
  GLuint floor_texture_gamma_corrected_;

  std::vector<glm::vec3> light_positions_;
  std::vector<glm::vec3> light_colors_;

  bool gamma_enabled_ = false;
  bool gamma_key_pressed_ = false;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
