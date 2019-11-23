#include "base/glfw_base.h"

#include <iostream>

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
      }
    )fs");
    screen_shader_.Create(
    R"vs(
      #version 330 core
      layout (location = 0) in vec2 aPos;
      layout (location = 1) in vec2 aTexCoords;

      out vec2 TexCoords;

      void main() {
        TexCoords = aTexCoords;
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
      }
    )vs",
    R"fs(
      #version 330 core
      out vec4 FragColor;

      in vec2 TexCoords;

      uniform sampler2D screenTexture;

      /*
      void main() {
        // vec3 col = texture(screenTexture, TexCoords).rgb;
        // FragColor = vec4(col, 1.0);

        // Inversion
        // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

        // Grayscale
        FragColor = texture(screenTexture, TexCoords);
        float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
        FragColor = vec4(average, average, average, 1.0);
      }
      */

      // Kernel effects
      const float offset = 1.0 / 300.0;

      void main() {
        vec2 offsets[9] = vec2[](
          vec2(-offset,  offset), // top-left
          vec2( 0.0f,    offset), // top-center
          vec2( offset,  offset), // top-right
          vec2(-offset,  0.0f),   // center-left
          vec2( 0.0f,    0.0f),   // center-center
          vec2( offset,  0.0f),   // center-right
          vec2(-offset, -offset), // bottom-left
          vec2( 0.0f,   -offset), // bottom-center
          vec2( offset, -offset)  // bottom-right
        );

        // Sharpen
        float kernel[9] = float[](
          -1, -1, -1,
          -1,  9, -1,
          -1, -1, -1
        );
        // Blur
        // float kernel[9] = float[](
        //   1.0 / 16, 2.0 / 16, 1.0 / 16,
        //   2.0 / 16, 4.0 / 16, 2.0 / 16,
        //   1.0 / 16, 2.0 / 16, 1.0 / 16
        // );
        // Edge detection
        // float kernel[9] = float[](
        //   1,  1, 1,
        //   1, -8, 1,
        //   1,  1, 1
        // );

        vec3 sampleTex[9];
        for (int i = 0; i < 9; i++) {
          sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        for (int i = 0; i < 9; i++)
          col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
      }
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
    float quadVertices[] = {  // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
      // positions   // texCoords
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f,
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
    // screen quad VAO
    glGenVertexArrays(1, &quad_vao_);
    glGenBuffers(1, &quad_vbo_);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // load textures
    cube_texture_ = LoadTexture(MY_DIR "/textures/container.jpg");
    floor_texture_ = LoadTexture(MY_DIR "/textures/metal.png");

    // shader configuration
    shader_.Use();
    shader_.SetInt("texture1", 0);

    screen_shader_.Use();
    screen_shader_.SetInt("screenTexture", 0);

#ifdef __APPLE__
    /*
    For OSX retina device users, you may have to double the screen height and width when you create a framebuffer. Otherwise when you want to draw the screen using a framebuffer, some weird thing may occur (like I found only 1/4 of the previous content is rendered on the screen, which is the left corner).
    The reason of this might be that OSX uses 4 pixels to represent one under retina display. And GLFW aware of this problem while some other functions do not. So when you create a 800 x 600 window, it might actually be a 1600 x 1200 window...
    */
    GLsizei width = SCR_WIDTH * 2;
    GLsizei height = SCR_HEIGHT * 2;
#else
    GLsizei width = SCR_WIDTH;
    GLsizei height = SCR_HEIGHT;
#endif
    // framebuffer configuration
    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    // create a color attachment texture
    glGenTextures(1, &texture_color_buffer_);
    glBindTexture(GL_TEXTURE_2D, texture_color_buffer_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer_, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);  // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw as wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  bool IsGlfwDrawOverride(GlfwBase *) override { return true; }

  void OnGlfwDraw(GlfwBase *glfw) override {
    static CameraHelper2 &camera = CameraHelper2::Instance();
    // per-frame
    camera.OnFrame();
    // input
    camera.OnKeyEvent(glfw->GetWindow());

    // render
    // bind to framebuffer and draw scene as we normally would to color texture
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glEnable(GL_DEPTH_TEST);  // enable depth testing (is disabled for rendering screen-space quad)

    // make sure we clear the framebuffer's content
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

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);  // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    screen_shader_.Use();
    glBindVertexArray(quad_vao_);
    glBindTexture(GL_TEXTURE_2D, texture_color_buffer_);  // use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // glfw: swap buffers and poll IO events
    glfwSwapBuffers(glfw->GetWindow());
    glfwPollEvents();
  }

  void OnGlfwDestory(GlfwBase *) override {
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &cube_vao_);
    glDeleteVertexArrays(1, &plane_vao_);
    glDeleteVertexArrays(1, &quad_vao_);
    glDeleteBuffers(1, &cube_vbo_);
    glDeleteBuffers(1, &plane_vbo_);
    glDeleteBuffers(1, &quad_vbo_);
  }

 private:
  Shader shader_;
  Shader screen_shader_;
  GLuint cube_vao_;
  GLuint cube_vbo_;
  GLuint plane_vao_;
  GLuint plane_vbo_;
  GLuint quad_vao_;
  GLuint quad_vbo_;
  GLuint cube_texture_;
  GLuint floor_texture_;

  GLuint framebuffer_;
  GLuint texture_color_buffer_;
  GLuint rbo_;
};

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  CameraHelper2::Init(SCR_WIDTH, SCR_HEIGHT, Camera(glm::vec3(0.0f, 0.0f, 3.0f)));
  GlfwBase glfw_base;
  glfw_base.SetCallback(std::make_shared<GlfwBaseCallbackImpl>());
  return glfw_base.Run({SCR_WIDTH, SCR_HEIGHT, "GLFW Window"});
}
