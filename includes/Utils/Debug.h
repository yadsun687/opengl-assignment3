#ifndef DEBUG_H
#define DEBUG_H

#include <glm/glm.hpp>
#include <iostream>

class Debug {
public:
  static inline void logGLMVector(glm::vec3 v) {
    std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")";
  }
};

#endif
