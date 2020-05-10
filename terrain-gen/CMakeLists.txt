cmake_minimum_required(VERSION 3.0.0)
project(TerrainGenerator VERSION 0.1.0)

add_executable(TerrainGenerator main.cpp TerrainGenerator.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
