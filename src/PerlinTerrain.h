#ifndef PERLIN_TERRAIN_H
#define PERLIN_TERRAIN_H

#include <Godot.hpp>
#include <StaticBody.hpp>
#include <MeshInstance.hpp>
#include <CollisionShape.hpp>
#include <PackedScene.hpp>

namespace godot {

class PerlinTerrain : public StaticBody {
  GODOT_CLASS(PerlinTerrain, StaticBody)

//Public methods
public:
  static void _register_methods();

  void _input(InputEvent* event);
  void _init();
  void _ready();

private:

  void generateNewTerrain(const int seed = 0);

  const String MESH_INSTANCE_NAME = "MeshInstance";
  MeshInstance* mesh_;
  const String COLLISION_SHAPE_NAME = "CollisionShape";
  CollisionShape* collision_shape_;

  Ref<PackedScene> wall_scene_;

  //Parameters to be passed to TerrainGenerator

  int terrain_seed_;

  //Default height map grid width/height
  size_t x_size_;
  size_t z_size_;
  //Default size of square in perlin grid;
  size_t noise_freq_;
  //Max/Min height currently unused
  float max_height_;
  float min_height_;

  bool discretize_height_;
  bool limit_height_;

  float height_scale_;
  float height_step_size_;
  
};
}


#endif