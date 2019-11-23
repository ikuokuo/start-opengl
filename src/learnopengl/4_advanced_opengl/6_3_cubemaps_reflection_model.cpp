#include "base/glfw_base.h"

#include "common/camera.h"
#include "common/shader.h"
#include "common/texture.h"

#include "common/model.h"

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
      layout (location = 1) in vec3 aNormal;
      layout (location = 2) in vec2 aTexCoords;

      out vec3 Normal;
      out vec3 Position;
      out vec2 TexCoords;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        Normal = mat3(transpose(inverse(model))) * aNormal;
        Position = vec3(model * vec4(aPos, 1.0));
        TexCoords = aTexCoords;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      struct Material {
        sampler2D texture_diffuse1;
        sampler2D texture_specular1;
        sampler2D texture_reflection1;
        samplerCube skybox;  // texture10
        float shininess;
      };
      struct DirLight {
        vec3 direction;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
      };

      in vec3 Normal;
      in vec3 Position;
      in vec2 TexCoords;

      uniform vec3 cameraPos;

      uniform Material material;
      uniform DirLight dirLight;

      vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

      void main() {
        // FragColor = texture(material.texture_diffuse1, TexCoords);

        // directional lighting
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(cameraPos - Position);
        vec3 result = CalcDirLight(dirLight, norm, viewDir);

        // reflection
        vec3 I = normalize(Position - cameraPos);
        vec3 R = reflect(I, normalize(Normal));
        result += texture(material.skybox, R).rgb * vec3(texture(material.texture_reflection1, TexCoords)).rgb;

        FragColor = vec4(result, 1.0);
      }

      // calculates the color when using a directional light.
      vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
        vec3 lightDir = normalize(-light.direction);
        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // combine results
        vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
        vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
        return (ambient + diffuse + specular);
      }
    )fs");
    skybox_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;

      out vec3 TexCoords;

      uniform mat4 projection;
      uniform mat4 view;

      void main() {
        TexCoords = aPos;
        vec4 pos = projection * view * vec4(aPos, 1.0);
        gl_Position = pos.xyww;
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      in vec3 TexCoords;

      uniform samplerCube skybox;

      void main() {
        FragColor = texture(skybox, TexCoords);
      }
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float skyboxVertices[] = {
      // positions
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,
    };

    // skybox VAO
    glGenVertexArrays(1, &skybox_vao_);
    glGenBuffers(1, &skybox_vbo_);
    glBindVertexArray(skybox_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // model_.Create(MY_DIR "/objects/nanosuit/nanosuit.obj");
    model_.Create(MY_DIR "/objects/nanosuit_reflection/nanosuit.obj");

    // load textures
    std::vector<std::string> faces{
      MY_DIR "/textures/skybox/right.jpg",
      MY_DIR "/textures/skybox/left.jpg",
      MY_DIR "/textures/skybox/top.jpg",
      MY_DIR "/textures/skybox/bottom.jpg",
      MY_DIR "/textures/skybox/front.jpg",
      MY_DIR "/textures/skybox/back.jpg",
    };
    cubemap_texture_ = LoadCubemap(faces);

    // shader configuration
    shader_.Use();
    shader_.SetInt("material.skybox", 10);

    skybox_shader_.Use();
    skybox_shader_.SetInt("skybox", 0);
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
    shader_.SetVec3("cameraPos", camera.GetCamera().Position);
    shader_.SetFloat("material.shininess", 32.0f);
    // directional light
    shader_.SetVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader_.SetVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
    shader_.SetVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
    shader_.SetVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

    // draw scene as normal
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetPerspectiveMatrix();
    shader_.SetMat4("view", view);
    shader_.SetMat4("projection", projection);
    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    shader_.SetMat4("model", model);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture_);
    model_.Draw(shader_);

    // draw skybox as last
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    skybox_shader_.Use();
    view = glm::mat4(glm::mat3(camera.GetViewMatrix()));  // remove translation from the view matrix
    skybox_shader_.SetMat4("view", view);
    skybox_shader_.SetMat4("projection", projection);
    // skybox cube
    glBindVertexArray(skybox_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);  // set depth function back to default

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &skybox_vao_);
    glDeleteBuffers(1, &skybox_vbo_);
  }

 private:
  Shader shader_;
  Shader skybox_shader_;
  GLuint skybox_vao_;
  GLuint skybox_vbo_;
  GLuint cubemap_texture_;
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
