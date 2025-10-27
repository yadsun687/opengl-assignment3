#ifndef PLAYER_H
#define PLAYER_H

#include <filesystem.h>
#include <glm/glm.hpp>
#include <model.h>

const float MODEL_SCALE = 0.1f;
const glm::vec3 CAMERA_OFFSET = {0, 0.6, 0};

//[Model from]
// Witch by Quaternius [CC-BY] (https://creativecommons.org/licenses/by/3.0/)
// via Poly Pizza (https://poly.pizza/m/QBEOV9ZUT8)
const string MODEL_PATH = "resources/models/player/Witch_triangulated3.fbx";

class Player {
public:
  int health = 20;

  glm::vec3 m_position;
  float m_movement_speed = 3.0f;
  Model m_model;
  float m_timeSinceStarted;
  glm::vec3 m_targetPosition;

  // facing angle in radian (-PI/2 - PI/2)
  float m_modelAngle;
  float m_targetModelAngle;
  float m_rotationSpeed;

  Player(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
      : m_model(FileSystem::getPath(MODEL_PATH).c_str()), m_modelAngle(0.0f),
        m_targetModelAngle(0.0f), m_rotationSpeed(glm::pi<float>() * 4.0f) {
    m_position = position;
    m_targetPosition = m_position;
  }
  ~Player() {};

  void moveToward(glm::vec3 direction, float deltaTime) {
    m_position += direction * (m_movement_speed * deltaTime);
  }

  void moveTo(glm::vec3 position) { m_position = position; }
};

#endif
