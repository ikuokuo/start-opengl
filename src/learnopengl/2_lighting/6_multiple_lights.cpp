#include "base/glfw_base.h"

#include <iostream>

#include "common/camera.h"
#include "common/shader.h"
#include "common/texture.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

class GlfwBaseCallbackImpl : public GlfwBaseCallback {
 public:
  GlfwBaseCallbackImpl() {
  }

  bool IsWindowCreatedOverride(GlfwBase *, GLFWwindow *) override { return true; };

  void OnGlfwInit(GlfwBase *glfw) override {
    CameraHelper2::glfw_init(glfw->GetWindow(), true);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    lighting_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 1) in vec3 aNormal;
      layout (location = 2) in vec2 aTexCoords;

      out vec3 FragPos;
      out vec3 Normal;
      out vec2 TexCoords;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoords = aTexCoords;

        gl_Position = projection * view * vec4(FragPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      struct Material {
        sampler2D diffuse;
        sampler2D specular;
        float shininess;
      };

      struct DirLight {
        vec3 direction;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
      };

      struct PointLight {
        vec3 position;

        float constant;
        float linear;
        float quadratic;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
      };

      struct SpotLight {
        vec3 position;
        vec3 direction;
        float cutOff;
        float outerCutOff;

        float constant;
        float linear;
        float quadratic;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
      };

      #define NR_POINT_LIGHTS 4

      in vec3 FragPos;
      in vec3 Normal;
      in vec2 TexCoords;

      uniform vec3 viewPos;
      uniform DirLight dirLight;
      uniform PointLight pointLights[NR_POINT_LIGHTS];
      uniform SpotLight spotLight;
      uniform Material material;

      // function prototypes
      vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
      vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
      vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

      void main() {
        // properties
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);

        // phase 1: directional lighting
        vec3 result = CalcDirLight(dirLight, norm, viewDir);
        // phase 2: point lights
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
          result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        // phase 3: spot light
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

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
        vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        return (ambient + diffuse + specular);
      }

      // calculates the color when using a point light.
      vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
        vec3 lightDir = normalize(light.position - fragPos);
        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        // combine results
        vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        return (ambient + diffuse + specular);
      }

      // calculates the color when using a spot light.
      vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
        vec3 lightDir = normalize(light.position - fragPos);
        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        // spotlight intensity
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        // combine results
        vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        ambient *= attenuation * intensity;
        diffuse *= attenuation * intensity;
        specular *= attenuation * intensity;
        return (ambient + diffuse + specular);
      }
    )fs");

    lamp_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec3 aPos;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      void main() {
        FragColor = vec4(1.0); // set alle 4 vector values to 1.0
      }
    )fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
       0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
      -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
      -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
       0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
       0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    };
    // positions all containers
    cube_positions_ = {
      glm::vec3( 0.0f,  0.0f,  0.0f),
      glm::vec3( 2.0f,  5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f),
      glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3( 2.4f, -0.4f, -3.5f),
      glm::vec3(-1.7f,  3.0f, -7.5f),
      glm::vec3( 1.3f, -2.0f, -2.5f),
      glm::vec3( 1.5f,  2.0f, -2.5f),
      glm::vec3( 1.5f,  0.2f, -1.5f),
      glm::vec3(-1.3f,  1.0f, -1.5f),
    };
    // positions of the point lights
    point_light_positions_ = {
      glm::vec3( 0.7f,  0.2f,  2.0f),
      glm::vec3( 2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f,  2.0f, -12.0f),
      glm::vec3( 0.0f,  0.0f, -3.0f),
    };

    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &cube_vao_);
    glGenBuffers(1, &vbo_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cube_vao_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    glGenVertexArrays(1, &light_vao_);
    glBindVertexArray(light_vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load textures (we now use a utility function to keep the code more organized)
    diffuse_map_ = LoadTexture(MY_DIR "/textures/container2.png");
    specular_map_ = LoadTexture(MY_DIR "/textures/container2_specular.png");

    // shader configuration
    lighting_shader_.Use();
    lighting_shader_.SetInt("material.diffuse", 0);
    lighting_shader_.SetInt("material.specular", 1);
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

    // be sure to activate shader when setting uniforms/drawing objects
    lighting_shader_.Use();
    lighting_shader_.SetVec3("viewPos", camera.GetCamera().Position);
    lighting_shader_.SetFloat("material.shininess", 32.0f);

    /*
      Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
      the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
      by defining light types as classes and set their values in there, or by using a more efficient uniform approach
      by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
    */
    // directional light
    lighting_shader_.SetVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    lighting_shader_.SetVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    lighting_shader_.SetVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    lighting_shader_.SetVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    lighting_shader_.SetVec3("pointLights[0].position", point_light_positions_[0]);
    lighting_shader_.SetVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    lighting_shader_.SetVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    lighting_shader_.SetVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    lighting_shader_.SetFloat("pointLights[0].constant", 1.0f);
    lighting_shader_.SetFloat("pointLights[0].linear", 0.09);
    lighting_shader_.SetFloat("pointLights[0].quadratic", 0.032);
    // point light 2
    lighting_shader_.SetVec3("pointLights[1].position", point_light_positions_[1]);
    lighting_shader_.SetVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    lighting_shader_.SetVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    lighting_shader_.SetVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    lighting_shader_.SetFloat("pointLights[1].constant", 1.0f);
    lighting_shader_.SetFloat("pointLights[1].linear", 0.09);
    lighting_shader_.SetFloat("pointLights[1].quadratic", 0.032);
    // point light 3
    lighting_shader_.SetVec3("pointLights[2].position", point_light_positions_[2]);
    lighting_shader_.SetVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    lighting_shader_.SetVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    lighting_shader_.SetVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    lighting_shader_.SetFloat("pointLights[2].constant", 1.0f);
    lighting_shader_.SetFloat("pointLights[2].linear", 0.09);
    lighting_shader_.SetFloat("pointLights[2].quadratic", 0.032);
    // point light 4
    lighting_shader_.SetVec3("pointLights[3].position", point_light_positions_[3]);
    lighting_shader_.SetVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    lighting_shader_.SetVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    lighting_shader_.SetVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    lighting_shader_.SetFloat("pointLights[3].constant", 1.0f);
    lighting_shader_.SetFloat("pointLights[3].linear", 0.09);
    lighting_shader_.SetFloat("pointLights[3].quadratic", 0.032);
    // spotLight
    lighting_shader_.SetVec3("spotLight.position", camera.GetCamera().Position);
    lighting_shader_.SetVec3("spotLight.direction", camera.GetCamera().Front);
    lighting_shader_.SetVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lighting_shader_.SetVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lighting_shader_.SetVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lighting_shader_.SetFloat("spotLight.constant", 1.0f);
    lighting_shader_.SetFloat("spotLight.linear", 0.09);
    lighting_shader_.SetFloat("spotLight.quadratic", 0.032);
    lighting_shader_.SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    lighting_shader_.SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    // view/projection transformations
    glm::mat4 projection = camera.GetPerspectiveMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    lighting_shader_.SetMat4("projection", projection);
    lighting_shader_.SetMat4("view", view);

    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    lighting_shader_.SetMat4("model", model);

    // bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_map_);
    // bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_map_);

    // render containers
    glBindVertexArray(cube_vao_);
    for (std::size_t i = 0, n = cube_positions_.size(); i < n; i++) {
      // calculate the model matrix for each object and pass it to shader before drawing
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cube_positions_[i]);
      float angle = 20.0f * i;
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      lighting_shader_.SetMat4("model", model);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // also draw the lamp object
    lamp_shader_.Use();
    lamp_shader_.SetMat4("projection", projection);
    lamp_shader_.SetMat4("view", view);
    // we now draw as many light bulbs as we have point lights.
    glBindVertexArray(light_vao_);
    for (unsigned int i = 0; i < 4; i++) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, point_light_positions_[i]);
      model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
      lamp_shader_.SetMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }


    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &cube_vao_);
    glDeleteVertexArrays(1, &light_vao_);
    glDeleteBuffers(1, &vbo_);
  }

 private:
  Shader lighting_shader_;
  Shader lamp_shader_;
  GLuint cube_vao_;
  GLuint light_vao_;
  GLuint vbo_;
  GLuint diffuse_map_;
  GLuint specular_map_;
  std::vector<glm::vec3> cube_positions_;
  std::vector<glm::vec3> point_light_positions_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 5.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
