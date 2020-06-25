#ifndef TERRAIN_GENERATOR_H
#define TERRAIN_GENERATOR_H

#include <vector>
#include <array>
#include <string>
#include <optional>

namespace TerrainGen{
  using HeightMap = std::vector<std::vector<float>>;
  using TGenSeed = int;
  struct Config {
    size_t h_size = 100;
    size_t v_size = 100;
    //How many h / v units (above sizes) to include in one gradient calculation
    size_t gradient_grid_size = 12;
    //Scaling parameter
    float height_scale = 10.f;

    std::optional<float> min_height;
    std::optional<float> max_height;
    //step size for different allowed heights (discretize)
    std::optional<float> height_step_size;
  };

  std::optional<HeightMap> generateTerrain(TGenSeed seed, const Config& cfg);
}

#endif