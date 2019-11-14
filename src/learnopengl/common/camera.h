#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum CameraMovement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
 public:
  // Camera Attributes
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  // Euler Angles
  float Yaw;
  float Pitch;
  // Camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  // Constructor with vectors
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
         float yaw = YAW, float pitch = PITCH)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    UpdateCameraVectors();
  }
  // Constructor with scalar values
  Camera(float posX, float posY, float posZ,
         float upX, float upY, float upZ,
         float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    UpdateCameraVectors();
  }

  // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
  }

  // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
  void ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)   Position += Front * velocity;
    if (direction == BACKWARD)  Position -= Front * velocity;
    if (direction == LEFT)      Position -= Right * velocity;
    if (direction == RIGHT)     Position += Right * velocity;
  }

  // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
  void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
      if (Pitch > 89.0f)  Pitch = 89.0f;
      if (Pitch < -89.0f) Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    UpdateCameraVectors();
  }

  // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
  void ProcessMouseScroll(float yoffset) {
    if (Zoom >= 1.0f && Zoom <= 45.0f)  Zoom -= yoffset;
    if (Zoom <= 1.0f)                   Zoom = 1.0f;
    if (Zoom >= 45.0f)                  Zoom = 45.0f;
  }

 private:
  // Calculates the front vector from the Camera's (updated) Euler Angles
  void UpdateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up    = glm::normalize(glm::cross(Right, Front));
  }
};


class CameraHelperInterface {
 public:
  virtual ~CameraHelperInterface() = default;
  virtual Camera &GetCamera() = 0;
  virtual glm::mat4 GetPerspectiveMatrix(float z_near = 0.1f, float z_far = 100.0f) = 0;
  virtual glm::mat4 GetViewMatrix() = 0;
  virtual void OnFrame() = 0;
  virtual void OnKeyEvent(GLFWwindow *) = 0;
  virtual void OnMouseEvent(GLFWwindow *, double xpos, double ypos) = 0;
  virtual void OnScrollEvent(GLFWwindow *, double xoffset, double yoffset) = 0;
};

class CameraHelper : public CameraHelperInterface {
 public:
  CameraHelper(int scr_width, int scr_height,
               Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f)))
    : scr_width_(scr_width),
      scr_height_(scr_height),
      camera_(std::move(camera)),
      first_mouse_(true),
      last_x_(scr_width / 2.0f),
      last_y_(scr_height / 2.0f) {
  }

  Camera &GetCamera() override { return camera_; }

  glm::mat4 GetPerspectiveMatrix(float z_near = 0.1f, float z_far = 100.0f) override {
    return glm::perspective(glm::radians(camera_.Zoom),
        (float)scr_width_ / (float)scr_height_, z_near, z_far);
  }

  glm::mat4 GetViewMatrix() override {
    return camera_.GetViewMatrix();
  }

  void OnFrame() override {
    float current_frame = glfwGetTime();
    delta_time_ = current_frame - last_frame_;
    last_frame_ = current_frame;
  }

  void OnKeyEvent(GLFWwindow *window) override {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera_.ProcessKeyboard(FORWARD, delta_time_);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera_.ProcessKeyboard(BACKWARD, delta_time_);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera_.ProcessKeyboard(LEFT, delta_time_);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera_.ProcessKeyboard(RIGHT, delta_time_);
  }

  // glfw: whenever the mouse moves
  void OnMouseEvent(GLFWwindow *, double xpos, double ypos) override {
    if (first_mouse_) {
      last_x_ = xpos;
      last_y_ = ypos;
      first_mouse_ = false;
    }

    float xoffset = xpos - last_x_;
    float yoffset = last_y_ - ypos; // reversed since y-coordinates go from bottom to top

    last_x_ = xpos;
    last_y_ = ypos;

    camera_.ProcessMouseMovement(xoffset, yoffset);
  }

  // glfw: whenever the mouse scroll wheel scrolls
  void OnScrollEvent(GLFWwindow *, double xoffset, double yoffset) override {
    (void)xoffset;
    camera_.ProcessMouseScroll(yoffset);
  }

 private:
  int scr_width_;
  int scr_height_;
  // camera
  Camera camera_;
  bool first_mouse_;
  float last_x_;
  float last_y_;
  // timing
  float delta_time_ = 0.0f; // time between current frame and last frame
  float last_frame_ = 0.0f;
};

class CameraHelper2 : public CameraHelperInterface {
 private:
  CameraHelper2(int scr_width, int scr_height, Camera camera)
    : helper_(scr_width, scr_height, std::move(camera)) {
    if (scr_width == 0 || scr_height == 0)
      throw std::runtime_error{"CameraHelper2 not initialized"};
  }

  static CameraHelper2 &InstanceImpl(
      int scr_width = 0, int scr_height = 0,
      Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f))) {
    static CameraHelper2 instance(scr_width, scr_height, std::move(camera));
    return instance;
  }

 public:
  static void Init(
      int scr_width, int scr_height,
      Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f))) {
    InstanceImpl(scr_width, scr_height, std::move(camera));
  }
  static CameraHelper2 &Instance() {
    return InstanceImpl();
  }

  Camera &GetCamera() override { return helper_.GetCamera(); }

  glm::mat4 GetPerspectiveMatrix(float z_near = 0.1f, float z_far = 100.0f) override {
    return helper_.GetPerspectiveMatrix(z_near, z_far);
  }

  glm::mat4 GetViewMatrix() override {
    return helper_.GetViewMatrix();
  }

  void OnFrame() override {
    helper_.OnFrame();
  }

  void OnKeyEvent(GLFWwindow *window) override {
    helper_.OnKeyEvent(window);
  }

  void OnMouseEvent(GLFWwindow *window, double xpos, double ypos) override {
    helper_.OnMouseEvent(window, xpos, ypos);
  }

  void OnScrollEvent(GLFWwindow *window, double xoffset, double yoffset) override {
    helper_.OnScrollEvent(window, xoffset, yoffset);
  }

  static void glfw_init(GLFWwindow *window, bool glew_init = true) {
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetCursorPosCallback(window, glfw_mouse_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glew_init) CameraHelper2::glew_init();
  }
  static void glew_init() {
    // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      return;
    }
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;
  }
  // glfw: whenever the window size changed (by OS or user resize) this callback function executes
  static void glfw_framebuffer_size_callback(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
  }
  // glfw: whenever the mouse moves, this callback is called
  static void glfw_mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    CameraHelper2::Instance().OnMouseEvent(window, xpos, ypos);
  }
  // glfw: whenever the mouse scroll wheel scrolls, this callback is called
  static void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    CameraHelper2::Instance().OnScrollEvent(window, xoffset, yoffset);
  }

 private:
  CameraHelper helper_;
};
