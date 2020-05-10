#include <iostream>
#include <fstream>
#include "TerrainGenerator.h"

int main(int, char**) {
    TerrainGenerator terr_gen;

    TerrainGenerator::HeightMap map = terr_gen.generateTerrain(4);

    auto outstr = TerrainGenerator::csvFromHeightMap(map);

    std::ofstream ofs("out.csv", std::ios_base::trunc);
    if(ofs){
        ofs << outstr.c_str();
    }
    ofs.close();
}
