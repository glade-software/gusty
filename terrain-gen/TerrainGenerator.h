#ifndef TERRAIN_GENERATOR_H
#define TERRAIN_GENERATOR_H

#include <vector>
#include <array>
#include <string>

class TerrainGenerator{
public: 
  using HeightMap = std::vector<std::vector<float>>;
  
  TerrainGenerator();
  HeightMap generateTerrain(const int seed = 0);

  void setHeight(const size_t height);
  void setWidth(const size_t width);
  void setGradientGridSize(const size_t grid_size);

  static std::string csvFromHeightMap(const HeightMap& height_map);

private:
  //Make class non-copyable
  TerrainGenerator(const TerrainGenerator&) = delete;
  TerrainGenerator& operator=(const TerrainGenerator&) = delete;

  static constexpr size_t PERMUTATION_TABLE_SIZE = 512;
  using PermutationArr = std::array<uint8_t, PERMUTATION_TABLE_SIZE>;

  PermutationArr calcPermutations(const int seed = 0);
  float grad(const uint8_t hash, const float x, const float y);
  float lerp(const float pt1, const float pt2, const float interpolate_amt);
  float fade(const float t);

  // Member variables for configuration
  //Give them default values, use setters for configuring
  size_t h_size_ = 100;
  size_t v_size_ = 100;

  //How many h / v units (above sizes) to include in one gradient calculation
  size_t gradient_grid_size_ = 12;

};

#endif