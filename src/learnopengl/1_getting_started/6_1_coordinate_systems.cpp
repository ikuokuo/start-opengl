#include "base/glfw_base.h"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common/shader.h"

const unsigned int SCR_WIDTH = 400;
const unsigned int SCR_HEIGHT = 300;

class GlfwBaseCallbackImpl : public GlfwBaseCallback {
 public:
  GlfwBaseCallbackImpl() : shader_(nullptr) {
  }

  void OnGlfwInit(GlfwBase *) override {
    // Initialize GLEW
    glewExperimental = true;  // Needed in core profile
    if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      return;
    }
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

    // Shader sources
    const char *vertexShaderSource = R"glsl(
      #version 330 core
      layout (location = 0) in vec3 aPos;
      layout (location = 1) in vec2 aTexCoord;

      out vec2 TexCoord;

      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
      })glsl";
    const char *fragmentShaderSource = R"glsl(
      #version 330 core
      out vec4 FragColor;

      in vec2 TexCoord;

      // texture sampler
      uniform sampler2D texture1;
      uniform sampler2D texture2;

      void main() {
        // linearly interpolate between both textures (80% container, 20% awesomeface)
        FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
      })glsl";

    // build and compile our shader program
    shader_ = std::unique_ptr<Shader>(new Shader(
      vertexShaderSource, fragmentShaderSource));

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
      // positions          // texture coords
       0.5f,  0.5f, 0.0f,   1.0f, 1.0f,  // top right
       0.5f, -0.5f, 0.0f,   1.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,  // bottom left
      -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // top left
    };
    unsigned int indices[] = {
      0, 1, 3,  // first triangle
      1, 2, 3,  // second triangle
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture

    // texture 1
    glGenTextures(1, &texture1_);
    glBindTexture(GL_TEXTURE_2D, texture1_);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(MY_DIR "/textures/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // texture 2
    glGenTextures(1, &texture2_);
    glBindTexture(GL_TEXTURE_2D, texture2_);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load(MY_DIR "/textures/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data) {
      // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    shader_->Use();
    shader_->SetInt("texture1", 0);
    shader_->SetInt("texture2", 1);
  }

  void OnGlfwDraw(GlfwBase *glfw) override {
    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2_);

    // activate shader
    shader_->Use();

    // create transformations
    glm::mat4 model         = glm::mat4(1.0f);  // make sure to initialize matrix to identity matrix first
    glm::mat4 view          = glm::mat4(1.0f);
    glm::mat4 projection    = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    // retrieve the matrix uniform locations
    unsigned int modelLoc = glGetUniformLocation(shader_->ID, "model");
    unsigned int viewLoc  = glGetUniformLocation(shader_->ID, "view");
    // pass them to the shaders (3 different ways)
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
    shader_->SetMat4("projection", projection);

    // render container
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

 private:
  std::unique_ptr<Shader> shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint ebo_;
  GLuint texture1_;
  GLuint texture2_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
