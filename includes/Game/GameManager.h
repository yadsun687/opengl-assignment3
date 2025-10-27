#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H
#include <vector>
#include <iostream>
#include <mutex>
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <random>
#include <memory>
#include <Game/Player.h>

struct Projectile {
  glm::vec3 position;
  glm::vec3 velocity;
  float lifetime;
  float timeAlive;
  int damage = 20;
  Projectile(glm::vec3 pos, glm::vec3 vel, float lt, float ta)
      : position(pos), velocity(vel), lifetime(lt), timeAlive(ta) {
    std::cout << "Projectile created at (" << pos.x << ", " << pos.y << ", "
              << pos.z << ")\n";
  }
};

struct Enemy {
  glm::vec3 position;
  float rotation = 0.0f;
  float speed;
  int health;
  int damage;
  Enemy(glm::vec3 pos, float spd, int hp, int dmg)
      : position(pos), speed(spd), health(hp), damage(dmg) {
    std::cout << "Enemy created at (" << pos.x << ", " << pos.y << ", " << pos.z
              << ")\n";
  }
  ~Enemy() {}
};
const float PROJECTILE_SPEED = 6.0f;
const int MAX_PROJECTILE_POOL_SIZE = 30;
const int PROJECTILE_LIFTTIME = 4;

const int MAX_ENEMY_POOL_SIZE = 10;
const float SPAWN_INTERVAL = 2.0f; // in seconds

class GameManager {
private:
  // Static pointer to the GameManager instance
  inline static GameManager *instancePtr = nullptr;
  std::vector<std::shared_ptr<Projectile>> projectiles;
  glm::vec3 boundaryMin;
  glm::vec3 boundaryMax;
  int freeProjectileIdx = 0;
  float spawnCounter = 0.0f;

  std::vector<std::shared_ptr<Enemy>> enemies;
  int freeEnemyIdx = 0;

  // player
  Player player;

  // Random number generation
  std::mt19937 rng;
  std::uniform_real_distribution<float> posXDist;
  std::uniform_real_distribution<float> posZDist;
  std::uniform_int_distribution<int> sideDist;
  std::uniform_real_distribution<float> velocityMagnitudeDist;
  std::uniform_real_distribution<float> angleDist;

  // Mutex to ensure thread safety
  inline static std::mutex mtx;

  GameManager()
      : player(glm::vec3(-3.0f, 0.0f, 3.0f)),
        boundaryMin(glm::vec3(-10.0f, 0.0f, -10.0f)),
        boundaryMax(glm::vec3(10.0f, 0.0f, 10.0f)), rng(std::random_device{}()),
        posXDist(boundaryMin.x, boundaryMax.x),
        posZDist(boundaryMin.z, boundaryMax.z),
        sideDist(0, 3), // 4 sides: 0 = left, 1 = right, 2 = front, 3 = back
        velocityMagnitudeDist(5.0f, 10.0f),
        angleDist(0.0f, 2.0f * glm::pi<float>()) {
    projectiles = std::vector<std::shared_ptr<Projectile>>(
        MAX_PROJECTILE_POOL_SIZE, nullptr);
    enemies = std::vector<std::shared_ptr<Enemy>>(MAX_ENEMY_POOL_SIZE, nullptr);
  }

  glm::vec3 getRandomPosition(int side) {
    float x, z;

    switch (side) {
    case 0: // Left side: x = boundaryMin.x
      x = boundaryMin.x;
      z = posZDist(rng);
      break;
    case 1: // Right side: x = boundaryMax.x
      x = boundaryMax.x;
      z = posZDist(rng);
      break;
    case 2: // Front side: z = boundaryMin.z
      x = posXDist(rng);
      z = boundaryMin.z;
      break;
    case 3: // Back side: z = boundaryMax.z
      x = posXDist(rng);
      z = boundaryMax.z;
      break;
    default:
      x = boundaryMin.x;
      z = boundaryMin.z;
      break;
    }
    float y = boundaryMin.y; // Keep y at boundary level (0.0)
    return glm::vec3(x, y, z);
  }

  glm::vec3 getRandomHorizontalVelocity(int side) {
    float magnitude = velocityMagnitudeDist(rng);
    float angle;

    // Determine angle range based on which side the projectile spawns from
    // This ensures the projectile moves INTO the boundary area
    switch (side) {
    case 0: // Left side: move right (need +x component)
      // Angle range: [-PI/2, PI/2] (pointing generally toward +x)
      angle = -glm::pi<float>() / 2 + angleDist(rng) * glm::pi<float>();
      break;
    case 1: // Right side: move left (need -x component)
      // Angle range: [PI/2, 3*PI/2] (pointing generally toward -x)
      angle = glm::pi<float>() / 2 + angleDist(rng) * glm::pi<float>();
      break;
    case 2: // Front side: move back (need +z component)
      // Angle range: [0, PI] (pointing generally toward +z)
      angle = angleDist(rng) * glm::pi<float>();
      break;
    case 3: // Back side: move front (need -z component)
      // Angle range: [PI, 2*PI] (pointing generally toward -z)
      angle = glm::pi<float>() + angleDist(rng) * glm::pi<float>();
      break;
    default:
      angle = 0.0f;
      break;
    }

    float vx = magnitude * std::cos(angle);
    float vy = 0.0f; // Horizontal velocity (no vertical component)
    float vz = magnitude * std::sin(angle);

    return glm::vec3(vx, vy, vz);
  }

public:
  GameManager(const GameManager &obj) = delete;

  const std::vector<std::shared_ptr<Projectile>> &getProjectiles() {
    return projectiles;
  }

  const std::vector<std::shared_ptr<Enemy>> &getEnemies() { return enemies; }

  // Static method to get the GameManager instance
  static GameManager *getInstance() {
    if (instancePtr == nullptr) {
      std::lock_guard<std::mutex> lock(mtx);
      if (instancePtr == nullptr) {
        instancePtr = new GameManager();
      }
    }
    return instancePtr;
  }

  Player &getPlayer() { return player; }

  void update(float deltaTime) {
    spawnCounter += deltaTime;

    // spawn every SPAWN_INTERVAL seconds
    if (spawnCounter >= SPAWN_INTERVAL) {
      spawnCounter = 0;

      int side = sideDist(rng);
      glm::vec3 randomPos = getRandomPosition(side);
      glm::vec3 randomVel = getRandomHorizontalVelocity(side);
      // generateProjectile(randomPos, randomVel, 10.0f);
      generateEnemy(randomPos, 2.0f, 20, 1);

      // generateProjectile(glm::vec3(0.0f, 0.0f, 0.0f),
      // glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);

      // std::cout << "[GameManager] Projectiles:\n[";
      // for (auto &projectile : projectiles) {
      //   if (projectile == nullptr) {
      //     std::cout << "-, ";
      //   } else {
      //     std::cout << "(" << projectile->position.x << ", "
      //               << projectile->position.y << ", " <<
      //               projectile->position.z
      //               << ")" << ", ";
      //   }
      // }
      // std::cout << "]\n";
      //       // Generate random position and velocity
    }

    updateProjectiles(deltaTime);
    updateEnemies(deltaTime);

    collisionDetection();
  }

  int generateProjectile(glm::vec3 position, glm::vec3 direction,
                         float speed = PROJECTILE_SPEED) {
    if (freeProjectileIdx == -1) {
      std::cout << "[GameManager] Projectile pool is full!\n";
      return -1;
    }
    int index = freeProjectileIdx;
    freeProjectileIdx = -1;
    auto newProjectile = std::make_shared<Projectile>(
        position + (direction * 0.1f), direction * speed, PROJECTILE_LIFTTIME,
        0.0f);

    projectiles[index] = newProjectile;
    std::cout << "[GameManager] Projectile created at index " << index
              << std::endl;
    return index;
  }

  /*
   * Iterate through the projectiles and update their positions and lifetimes
   * If expired projectiles found, mark its index for next projectile to be
   * created at
   */
  void updateProjectiles(float deltaTime) {
    for (size_t i = 0; i < projectiles.size(); i++) {

      if (projectiles[i] == nullptr ||
          projectiles[i]->timeAlive >= projectiles[i]->lifetime) {
        freeProjectileIdx = i;
        projectiles[i] = nullptr;
        continue;
      }
      projectiles[i]->position += projectiles[i]->velocity * deltaTime;
      projectiles[i]->timeAlive += deltaTime;
    }
  }

  int generateEnemy(glm::vec3 position, float speed, int health, int damage) {
    if (freeEnemyIdx == -1) {
      std::cout << "[GameManager] Enemies pool is full!\n";
      return -1;
    }
    int index = freeEnemyIdx;
    freeEnemyIdx = -1;
    auto newEnemy = std::make_shared<Enemy>(position, speed, health, damage);

    enemies[index] = newEnemy;
    std::cout << "[GameManager] Enemy created at index " << index << std::endl;
    return index;
  }

  // enemy follow player
  void updateEnemies(float deltaTime) {
    for (size_t i = 0; i < enemies.size(); i++) {

      if (enemies[i] == nullptr || enemies[i]->health <= 0) {
        freeEnemyIdx = i;
        enemies[i] = nullptr;
        continue;
      }
      if (glm::distance(enemies[i]->position, player.m_position) < 0.01f) {
        player.health = std::max(player.health - enemies[i]->damage, 0);
        freeEnemyIdx = i;
        enemies[i] = nullptr;
        std::cout << "[GameManager] Player HIT!!! (HP: " << player.health
                  << ")\n";
        continue;
      }

      glm::vec3 directionToPlayer =
          glm::normalize(player.m_position - enemies[i]->position);

      glm::vec2 horizontalDirection =
          glm::vec2(player.m_position.x - enemies[i]->position.x,
                    player.m_position.z - enemies[i]->position.z);
      enemies[i]->rotation =
          std::atan2(horizontalDirection.x, horizontalDirection.y);

      enemies[i]->position += directionToPlayer * enemies[i]->speed * deltaTime;
    }
  }

  void collisionDetection() {
    for (size_t i = 0; i < enemies.size(); i++) {
      if (enemies[i] == nullptr || enemies[i]->health <= 0)
        continue;
      for (size_t j = 0; j < projectiles.size(); j++) {
        if (projectiles[j] == nullptr || projectiles[j]->lifetime <= 0)
          continue;

        glm::vec3 enemiesPos =
            glm::vec3(enemies[i]->position.x, 0.0f, enemies[i]->position.z);
        glm::vec3 projectilePos = glm::vec3(projectiles[j]->position.x, 0.0f,
                                            projectiles[j]->position.z);
        if (glm::distance(enemiesPos, projectilePos) < 0.2f) {
          enemies[i]->health =
              std::max(enemies[i]->health - projectiles[j]->damage, 0);
          projectiles[j] = nullptr;
        }
      }
    }
  }
};
#endif
