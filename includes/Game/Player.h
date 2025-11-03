#ifndef PLAYER_H
#define PLAYER_H

#include <filesystem.h>
#include <glm/glm.hpp>
#include <model.h>
#include <Animation/animation.h>
#include <Animation/animator.h>

const float MODEL_SCALE = 0.1f;
const float ANIM_BLEND_RATE = 0.01f;

enum ACTION_STATE {
  IDLE,

  IDLE_RUNNING,
  RUNNING,
  RUNNING_IDLE,

  RUN_ROLLING,
  ROLLING,
  ROLLING_RUN,

  IDLE_PUNCH,
  PUNCH,
  PUNCH_IDLE
};

//[Model from]
// Witch by Quaternius [CC-BY] (https://creativecommons.org/licenses/by/3.0/)
// via Poly Pizza (https://poly.pizza/m/QBEOV9ZUT8)
const string MODEL_PATH = "resources/models/player/Witch_triangulated3.fbx";
const float MOVEMENT_SPEED = 5.0f;

class Player {
public:
  int health = 20;

  glm::vec3 m_position;
  Model m_model;
  float m_timeSinceStarted;
  glm::vec3 m_targetPosition;

  // facing angle in radian (-PI/2 - PI/2)
  float m_modelAngle;
  float m_targetModelAngle;
  float m_rotationSpeed;
  glm::vec3 facingDirection;

  bool isRoll = false;

  ACTION_STATE m_actionState = IDLE;
  std::vector<Animation> m_animations = {};
  Animator m_animator = nullptr;
  float m_blendAmount = 0.0f;

  Player(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
      : m_model(FileSystem::getPath(MODEL_PATH).c_str()), m_modelAngle(0.0f),
        m_targetModelAngle(0.0f), m_rotationSpeed(glm::pi<float>() * 4.0f),
        facingDirection(glm::vec3(0.0f, 0.0f, 0.0f)) {
    m_position = position;
    m_targetPosition = m_position;

    m_animations.emplace_back(
        Animation("resources/models/player/animationonly_Witch.fbx", &m_model,
                  4)); // idle
    m_animations.emplace_back(
        Animation("resources/models/player/animationonly_Witch.fbx", &m_model,
                  14)); // punch right
    m_animations.emplace_back(
        Animation("resources/models/player/animationonly_Witch.fbx", &m_model,
                  15)); // roll
    m_animations.emplace_back(
        Animation("resources/models/player/animationonly_Witch.fbx", &m_model,
                  16)); // run
    m_animator = Animator(&m_animations[0]);
  }
  ~Player() {};

  void moveToward(glm::vec3 direction, float deltaTime,
                  float movementSpeed = MOVEMENT_SPEED) {
    m_position += direction * (movementSpeed * deltaTime);
  }

  void moveTo(glm::vec3 position) { m_position = position; }

  void setActionState(ACTION_STATE state) { m_actionState = state; }
};

#endif
