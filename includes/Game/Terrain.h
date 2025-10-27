#ifndef TERRAIN_H
#define TERRAIN_H

#include <model.h>

class Terrain {
public:
  Model floorModel;
  Model wallModel;
  Terrain(string const &floor_path, string const &wall_path)
      : floorModel(floor_path), wallModel(wall_path) {};
  ~Terrain() {}
};
#endif
