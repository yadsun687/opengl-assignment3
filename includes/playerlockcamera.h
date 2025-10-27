#ifndef PLAYERLOCKCAMERA_H
#define PLAYERLOCKCAMERA_H

#include <Game/Player.h>
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// Default camera values
const float YAW = -90.0f;
const float PITCH = 45.0f; // point down toward floor
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float CAMERA_DISTANCE = 12.0f;

class PlayerLockCamera {
public:
  // camera Attributes
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  // euler Angles
  float Yaw;
  float Pitch;
  // camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;
  float m_hoverRadius;
  bool isTrackPlayer;
  float m_cameraDistance;

  // constructor with vectors
  PlayerLockCamera(Player &player, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                   float yaw = YAW, float pitch = PITCH)
      : Front(glm::vec3(0.0f, -1.0f, 0.0f)), MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY), Zoom(ZOOM), isTrackPlayer(false),
        m_cameraDistance(CAMERA_DISTANCE), player(player) {
    Position = player.m_position;
    playerLastPosition = player.m_position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    m_hoverRadius = 0.2;
    updateCameraPosition();
  }

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix() {
    return glm::lookAt(Position,
                       isTrackPlayer ? player.m_position : playerLastPosition,
                       WorldUp);
  }

  // processes input received from any keyboard-like input system. Accepts input
  // parameter in the form of camera defined ENUM (to abstract it from windowing
  // systems)
  void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
      Position += glm::vec3(0.0f, 0.0f, -1.0f) * velocity;
    if (direction == BACKWARD)
      Position -= glm::vec3(0.0f, 0.0f, -1.0f) * velocity;
    if (direction == LEFT)
      Position -= glm::vec3(1.0f, 0.0f, 0.0f) * velocity;
    if (direction == RIGHT)
      Position += glm::vec3(1.0f, 0.0f, 0.0f) * velocity;
  }

  // processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void ProcessMouseMovement(float xoffset, float yoffset,
                            GLboolean constrainPitch = true) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw = glm::mod(Yaw + xoffset, 360.0f);
    Pitch -= yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
      if (Pitch > 89.0f)
        Pitch = 89.0f;
      if (Pitch < -89.0f)
        Pitch = -89.0f;
    }

    updateCameraPosition();
  }

  // processes input received from a mouse scroll-wheel event. Only requires
  // input on the vertical wheel-axis
  void ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
      Zoom = 1.0f;
    if (Zoom > 45.0f)
      Zoom = 45.0f;
  }

  void updatePosition(glm::vec3 position) { Position = position; }

  void updateCameraPosition() {
    glm::vec3 cameraPosition;
    cameraPosition.x =
        m_cameraDistance * cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    cameraPosition.y = m_cameraDistance * sin(glm::radians(Pitch));
    cameraPosition.z =
        m_cameraDistance * sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position = player.m_position + cameraPosition;
    playerLastPosition =
        player.m_position; // keep track of last position to handle free camera
  }

private:
  Player &player;
  glm::vec3 playerLastPosition;
  // calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors() {
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(
        Front, WorldUp)); // normalize the vectors, because their length gets
                          // closer to 0 the more you look up or down which
                          // results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
  }
};
#endif
