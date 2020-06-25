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

// class TerrainGenerator{
// public: 
//   using HeightMap = std::vector<std::vector<float>>;
  
//   TerrainGenerator();
//   std::optional<HeightMap> generateTerrain(const int seed = 0);

//   void setHeight(const size_t height);
//   void setWidth(const size_t width);
//   void setGradientGridSize(const size_t grid_size);
//   void setHeightScale(const float scale);
//   void setHeightStepSize(const float step_size);

//   void setHeightLimits(const float min, const float max);

//   static std::string csvFromHeightMap(const HeightMap& height_map);

// private:
//   //Make class non-copyable
//   TerrainGenerator(const TerrainGenerator&) = delete;
//   TerrainGenerator& operator=(const TerrainGenerator&) = delete;

//   static constexpr size_t PERMUTATION_TABLE_SIZE = 512;
//   using PermutationArr = std::array<uint8_t, PERMUTATION_TABLE_SIZE>;

//   PermutationArr calcPermutations(const int seed = 0);
//   float grad(const uint8_t hash, const float x, const float y);
//   float lerp(const float pt1, const float pt2, const float interpolate_amt);
//   float fade(const float t);

//   float discretize(const float val, const float floor, const float step_size) const;

//   // Member variables for configuration
//   //Give them default values, use setters for configuring



// };

#endif