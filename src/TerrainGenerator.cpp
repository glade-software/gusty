#include "TerrainGenerator.h"
#include <sstream>
#include <random> 
#include <cmath>

namespace{
  static constexpr size_t MIN_H_SIZE = 3;
  static constexpr size_t MIN_V_SIZE = 3;
  static constexpr size_t PERMUTATION_TABLE_SIZE = 512;
  using PermutationArr = std::array<uint8_t, PERMUTATION_TABLE_SIZE>;

  //generates 512 random ints from 0-255, seeded
  PermutationArr calcPermutations(TerrainGen::TGenSeed seed){
    PermutationArr permutations;
     //TODO: confirm that this is actually deterministic.
     // If not, may need external dependency?
    std::minstd_rand0 rng(seed);

    for(auto& permutation : permutations){
      //random value from 0-3. only needs to be random between 4 different gradient vectors
      uint8_t rand_num = static_cast<uint8_t>(rng() % 256);
      permutation = rand_num;
    }

    return permutations;
  }

  float grad(const uint8_t hash, const float x, const float y){
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

  float lerp(const float pt1, const float pt2, float interpolate_amt){
    return pt1 + interpolate_amt * (pt2 - pt1);
  }

  float fade(const float t){
    return t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);
  }

  float discretize(const float val, const float floor, const float step_size){
    const float floor_dist = val - floor;
    const long steps_from_floor = std::lround(floor_dist / step_size);

    return floor + steps_from_floor * step_size;
  }
}


namespace TerrainGen{
  std::optional<HeightMap> generateTerrain(TGenSeed seed, const Config& cfg){
    //Check that parameters are valid
    if(cfg.v_size < MIN_V_SIZE || MIN_H_SIZE < 1 || 
      cfg.gradient_grid_size > cfg.v_size || cfg.gradient_grid_size > cfg.h_size){
        return std::nullopt;
    }

    if(cfg.min_height && cfg.max_height){
      if(*cfg.min_height >= *cfg.max_height){
        return std::nullopt;
      }
    }

    //generate permutation table from seed.
    const auto p_table = calcPermutations(seed);

    HeightMap ret(cfg.v_size);
    for(auto& h_vec : ret){
    h_vec.resize(cfg.h_size);
    }

    const float floor = cfg.min_height.value_or( -cfg.height_scale);

    //loop over each point in input grid
    for(size_t x = 0; x < cfg.h_size; x++){
      for(size_t y = 0; y < cfg.v_size; y++){

        //calc the unit square - xi, yi that sits on the "grid square"
        const auto xi = x - x % cfg.gradient_grid_size;
        const auto yi = y - y % cfg.gradient_grid_size;
        //xf, yf are coordinates relative to grid square
        const auto xf = static_cast<float>(x - xi);
        const auto yf = static_cast<float>(y - yi);

        //rand value for each of 4 pts of square
        //unruly but seems to work
        uint8_t aa, ab, ba, bb;
        aa = p_table[p_table[xi % 256] + yi % 256];
        ab = p_table[p_table[xi % 256] + (yi + cfg.gradient_grid_size) % 256];
        ba = p_table[p_table[(xi + cfg.gradient_grid_size) % 256] + yi % 256];
        bb = p_table[p_table[(xi + cfg.gradient_grid_size) % 256] + (yi + cfg.gradient_grid_size) % 256];

        float g_aa = grad(aa, xf, yf);
        float g_ba = grad(ba, xf - cfg.gradient_grid_size, yf);
        float g_ab = grad(ab, xf, yf - cfg.gradient_grid_size);
        float g_bb = grad(bb, xf - cfg.gradient_grid_size, yf - cfg.gradient_grid_size);

        float lerp_amt_x = fade(xf / cfg.gradient_grid_size);
        float lerp_amt_y = fade(yf / cfg.gradient_grid_size);

        float lerp1 = lerp(g_aa, g_ba, lerp_amt_x);
        float lerp2 = lerp(g_ab, g_bb, lerp_amt_x);
        
        //normalize the result to range of -1 to 1 by dividing by gradient size, and scale by configured factor
        float calculated_height = cfg.height_scale * lerp(lerp1, lerp2, lerp_amt_y) / cfg.gradient_grid_size;

        //for the below processing, would it make a significant difference to define const bool at the start of the function, 
        //instead of checking the if optionals have value each time? Probably insignificant, whatever is more readable

        //apply limits
        if(cfg.min_height.has_value()){
          if(calculated_height < *cfg.min_height){
            calculated_height = *cfg.min_height;
          }
        }
        if(cfg.max_height.has_value()){
          if(calculated_height > *cfg.max_height){
            calculated_height = *cfg.max_height;
          }
        }
        //fix number to step size
        if(cfg.height_step_size.has_value()){
          calculated_height = discretize(calculated_height, floor, *cfg.height_step_size);
        }
        ret[y][x] = calculated_height;
      }
    }

    return std::optional<HeightMap>{ret};
  }

  std::string csvFromHeightMap(const HeightMap& height_map){
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
}
