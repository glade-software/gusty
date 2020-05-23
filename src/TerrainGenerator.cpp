#include "TerrainGenerator.h"
#include <sstream>
#include <random>
#include <utility>
#include <cmath>

TerrainGenerator::TerrainGenerator(){

}

/**
 * For now, same config + same seed should generate the same map
*/
std::optional<TerrainGenerator::HeightMap> TerrainGenerator::generateTerrain(const int seed){

  //Check that parameters are valid
  if(v_size_ < MIN_V_SIZE || MIN_H_SIZE < 1 || 
     gradient_grid_size_ > v_size_ || gradient_grid_size_ > h_size_){
       return std::nullopt;
  }

  if(min_height_ && max_height_){
    if(*min_height_ >= *max_height_){
      return std::nullopt;
    }
  }

  //generate permutation table from seed.
  auto p_table = calcPermutations(seed);

  HeightMap ret(v_size_);
  for(auto& h_vec : ret){
   h_vec.resize(h_size_);
  }

  const float floor = min_height_.value_or( - height_scale_);

  //loop over each point in input grid
  for(size_t x = 0; x < h_size_; x++){
    for(size_t y = 0; y < v_size_; y++){

      //calc the unit square - xi, yi that sits on the "grid square"
      auto xi = x - x % gradient_grid_size_;
      auto yi = y - y % gradient_grid_size_;
      //xf, yf are coordinates relative to grid square
      float xf = x-xi;
      float yf = y-yi;

      //rand value for each of 4 pts of square
      //unruly but seems to work
      uint8_t aa, ab, ba, bb;
      aa = p_table[p_table[xi % 256] + yi % 256];
      ab = p_table[p_table[xi % 256] + (yi+gradient_grid_size_) % 256];
      ba = p_table[p_table[(xi+gradient_grid_size_) % 256] + yi % 256];
      bb = p_table[p_table[(xi+gradient_grid_size_) % 256] + (yi+gradient_grid_size_) % 256];

      float g_aa = grad(aa, xf, yf);
      float g_ba = grad(ba, xf - gradient_grid_size_, yf);
      float g_ab = grad(ab, xf, yf - gradient_grid_size_);
      float g_bb = grad(bb, xf - gradient_grid_size_, yf - gradient_grid_size_);

      float lerp_amt_x = fade(xf / gradient_grid_size_);
      float lerp_amt_y = fade(yf / gradient_grid_size_);

      float lerp1 = lerp(g_aa, g_ba, lerp_amt_x);
      float lerp2 = lerp(g_ab, g_bb, lerp_amt_x);
      
      //normalize the result to range of -1 to 1 by dividing by gradient size, and scale by configured factor
      float calculated_height = height_scale_ * lerp(lerp1, lerp2, lerp_amt_y) / gradient_grid_size_;

      //for the below processing, would it make a significant difference to define const bool at the start of the function, 
      //instead of checking the if optionals have value each time? Probably insignificant, whatever is more readable

      //apply limits
      if(min_height_.has_value()){
        if(calculated_height < *min_height_){
          calculated_height = *min_height_;
        }
      }
      if(max_height_.has_value()){
        if(calculated_height > *max_height_){
          calculated_height = *max_height_;
        }
      }
      //fix number to step size
      if(height_step_size_.has_value()){
        calculated_height = discretize(calculated_height, floor, *height_step_size_);
      }
      ret[y][x] = calculated_height;
    }
  }

  return std::optional<HeightMap>{ret};
}

float TerrainGenerator::grad(const uint8_t hash, const float x, const float y){
  //switch 2 lsb of hash
  //gives us one of four vectors (1, 1), (1, -1), (-1, 1), (-1, -1)
  //to dot product with (x, y)
  switch(hash & 0x3){
    case 0: return  x + y; // grad vec = ( 1, 1)
    case 1: return -x + y; // grad vec = (-1, 1)
    case 2: return  x - y; // grad vec = ( 1,-1)
    case 3: return -x - y; // grad vec = (-1,-1)
  }
}

float TerrainGenerator::lerp(const float pt1, const float pt2, float interpolate_amt){
  return pt1 + interpolate_amt * (pt2 - pt1);
}

float TerrainGenerator::fade(const float t){
  return t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);
}

float TerrainGenerator::discretize(const float val, const float floor, const float step_size) const{
  const float floor_dist = val - floor;
  const long steps_from_floor = std::lround(floor_dist / step_size);

  return floor + steps_from_floor * step_size;
}

//Configuration setters
void TerrainGenerator::setHeight(const size_t height){
  v_size_ = height;
}
void TerrainGenerator::setWidth(const size_t width){
  h_size_ = width;
}
void TerrainGenerator::setGradientGridSize(const size_t grid_size){
  gradient_grid_size_ = grid_size;
}

void TerrainGenerator::setHeightScale(const float height_scale){
  height_scale_ = height_scale;
}
void TerrainGenerator::setHeightStepSize(const float step_size){
  height_step_size_ = step_size;
}
void TerrainGenerator::setHeightLimits(const float min, const float max){
  min_height_ = min;
  max_height_ = max;
}

std::string TerrainGenerator::csvFromHeightMap(const HeightMap& height_map){
  std::stringstream ss;

  for(const auto& row: height_map){
    for(const auto& val: row){
      ss << val << ",";
    }
    //replace last comma with line ending
    ss.seekp(-1, ss.cur);
    ss << std::endl;
  }
  auto ret = ss.str();
  return ret;
}

//generates 512 random ints from 0-255, seeded
TerrainGenerator::PermutationArr TerrainGenerator::calcPermutations(const int seed){

  PermutationArr permutations;
  std::minstd_rand0 rng(seed);

  for(auto& permutation : permutations){
    //random value from 0-3. only needs to be random between 4 different gradient vectors
    uint8_t rand_num = static_cast<uint8_t>(rng() % 256);
    permutation = rand_num;
  }

  return permutations;
}