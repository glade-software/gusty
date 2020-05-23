#include "PerlinTerrain.h"
#include "TerrainGenerator.h"
#include <InputEvent.hpp>
#include <HeightMapShape.hpp>
#include <OS.hpp>
#include <SpatialMaterial.hpp>
#include <SurfaceTool.hpp>
#include <ArrayMesh.hpp>
#include <Mesh.hpp>

using namespace godot;

//Default height map grid width/height
static const size_t DEFAULT_X_SIZE = 100;
static const size_t DEFAULT_Z_SIZE = 100;
//Default size of square in perlin grid;
static const size_t DEFAULT_FREQ = 12;
//Max/Min height currently unused
static const float DEFAULT_MAX_HEIGHT = 10.0f;
static const float DEFAULT_MIN_HEIGHT = -10.0f;

void PerlinTerrain::_register_methods(){
  register_method("_ready", &PerlinTerrain::_ready);
  register_method("_input", &PerlinTerrain::_input);

  register_property<PerlinTerrain, size_t>("X Size", &PerlinTerrain::x_size_, DEFAULT_X_SIZE);
  register_property<PerlinTerrain, size_t>("Z Size", &PerlinTerrain::z_size_, DEFAULT_Z_SIZE);
  register_property<PerlinTerrain, size_t>("Noise Frequency", &PerlinTerrain::noise_freq_, DEFAULT_FREQ);
  register_property<PerlinTerrain, bool>("Limit Height", &PerlinTerrain::limit_height_, false);
  register_property<PerlinTerrain, float>("Max Height", &PerlinTerrain::max_height_, DEFAULT_MAX_HEIGHT);
  register_property<PerlinTerrain, float>("Min Height", &PerlinTerrain::min_height_, DEFAULT_MIN_HEIGHT);
  register_property<PerlinTerrain, int>("Random Seed", &PerlinTerrain::terrain_seed_, 0);
  register_property<PerlinTerrain, float>("Height Scale", &PerlinTerrain::height_scale_, 1.0f);
  register_property<PerlinTerrain, bool>("Discretize Height", &PerlinTerrain::discretize_height_, false);
  register_property<PerlinTerrain, float>("Height Step Size", &PerlinTerrain::height_step_size_, 1.0f);

  register_property<PerlinTerrain, Ref<PackedScene>>("Wall", &PerlinTerrain::wall_scene_, nullptr);
  register_signal<PerlinTerrain>("new_terrain_generated", "start_pos", GODOT_VARIANT_TYPE_VECTOR3);
}

void PerlinTerrain::_init(){
  x_size_ = DEFAULT_X_SIZE;
  z_size_ = DEFAULT_Z_SIZE;
  noise_freq_ = DEFAULT_FREQ;
  max_height_ = DEFAULT_MAX_HEIGHT;
  min_height_ = DEFAULT_MIN_HEIGHT;
  height_scale_ = 1.0f;
  height_step_size_ = 1.0f;
  discretize_height_ = false;
  limit_height_ = false;

  terrain_seed_ = 0;
  
  mesh_ = nullptr;
  collision_shape_ = nullptr;
}

void PerlinTerrain::_input(InputEvent* event){
  if(event->is_action_pressed("generate")){
    generateNewTerrain(terrain_seed_);
  }
}

void PerlinTerrain::_ready(){

  //Get child nodes that are needed
  mesh_ = Object::cast_to<MeshInstance>(get_node(NodePath(MESH_INSTANCE_NAME)));
  if(!mesh_){
    Godot::print("Terrain: Error finding mesh");
  }

  collision_shape_ = Object::cast_to<CollisionShape>(get_node(NodePath(COLLISION_SHAPE_NAME)));
  if(!collision_shape_){
    Godot::print("Terrain: error finding collision shape");
  }
}

void PerlinTerrain::generateNewTerrain(const int seed){
  Godot::print("generating terrain");
  TerrainGenerator tg;

  //Apply current settings
  tg.setHeight(z_size_);
  tg.setWidth(x_size_);
  tg.setGradientGridSize(noise_freq_);
  tg.setHeightScale(height_scale_);
  if(limit_height_){
    tg.setHeightLimits(min_height_, max_height_);
  }
  if(discretize_height_){
    tg.setHeightStepSize(height_step_size_);
  }

  //set max / min height once I have those options. 


  auto generated_map = tg.generateTerrain(seed);
  if(!generated_map){
    Godot::print("map generation failed, invalid parameters");
    return;
  }

  //height map is vec<vec<float>> - how to flatten this into a 1D vec for height map

  //width is the size of inner vector

  //depth is the size of outer vector

    // Ref<int> t;
  auto height_map_shape = Object::cast_to<HeightMapShape>(*collision_shape_->get_shape());
  if(!height_map_shape){
    Godot::print("Terrain: error finding height map shape");
    return;
  }

  //Delete walls that may have been left behind
  auto children = get_children();

  for(size_t i = 0; i < children.size(); i++){
    auto wall = Object::cast_to<StaticBody>(children[i]);
    if(wall){
      wall->queue_free();
    }
  }

  const auto map_width = generated_map.value()[0].size();
  const auto map_depth = generated_map.value().size();

  height_map_shape->set_map_width(map_width);
  height_map_shape->set_map_depth(map_depth);

  //Should consider efficiency of this method
  //If significant, can have TerrainGenerator return a PoolRealArray directly
  PoolRealArray map_data;
  for(const auto& row : generated_map.value()){
    //add row to RealArray
    for(const auto& val : row){
      map_data.push_back(val);
    }
  }
  height_map_shape->set_map_data(map_data);

  //Polygon drawing
  Ref<SpatialMaterial> material = SpatialMaterial::_new();
  material->set_albedo(Color(0.4, 0.9, 0.1, 1.0)); //Green

  Ref<SurfaceTool> surface_tool = SurfaceTool::_new();
  surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
  surface_tool->set_material(material);
  surface_tool->add_uv(Vector2(0,0));

  const auto x_scale = collision_shape_->get_scale().x;
  const auto z_scale = collision_shape_->get_scale().z;

  const auto start_x = - static_cast<float>(map_width - 1) * x_scale / 2.0f;
  const auto start_z = - static_cast<float>(map_depth - 1) * z_scale / 2.0f;

  Vector3 vertex_pos;

  PoolVector3Array vertices;
  vertices.resize(4);

  for(size_t i = 0; i < map_depth-1; i++){
    for(size_t j = 0; j < map_width-1; j++){
      //root is top left vertex of current square in the grid
      auto root_x = start_x + j * x_scale;
      auto root_z = start_z + i * z_scale;

      vertex_pos.x = root_x;
      vertex_pos.z = root_z;
      vertex_pos.y = map_data[map_width * i + j];
      vertices.set(0, vertex_pos);

      vertex_pos.x = root_x + x_scale;
      vertex_pos.z = root_z;
      vertex_pos.y = map_data[map_width * i + j+1];
      vertices.set(1, vertex_pos);

      vertex_pos.x = root_x + x_scale;
      vertex_pos.z = root_z + z_scale;
      //Going out of bounds here. make sure my indexes are all good
      // i = 0, j = 2449
      vertex_pos.y = map_data[map_width * (i+1) + j+1];
      vertices.set(2, vertex_pos);

      vertex_pos.x = root_x;
      vertex_pos.z = root_z + z_scale;
      vertex_pos.y = map_data[map_width * (i+1) + j];
      vertices.set(3, vertex_pos);

      // find steepest diagonal
      float slope02 = abs(vertices[0].y - vertices[2].y);
      float slope13 = abs(vertices[1].y - vertices[3].y);
      //flip draw order to keep consistent "concave" like slopes
      if(slope02 > slope13){
        vertices.insert(0, vertices[3]);
        vertices.remove(4);
      }

      surface_tool->add_triangle_fan(vertices);
    }
  }

  surface_tool->generate_normals();
  const auto generated_mesh = surface_tool->commit();
  mesh_->set_mesh(generated_mesh);

  if(wall_scene_.is_null()){
    Godot::print("Terrain: wall scene not specified");
  }

  //Set up walls
  auto* wall = Object::cast_to<Spatial>(wall_scene_->instance());

  //TODO: stretch wall height to min height.

  add_child(wall);
  // auto new_translation = wall->get_translation();
  // new_translation.z = - start_z;
  // wall->set_translation(new_translation);
  wall->translate({0,0, -start_z});
  // auto new_rotation = wall->get_rotation();
  // new_translation.z = - start_z;
  wall->rotate_y(Math_PI/2.0f);
  wall->set_scale({1, 1, start_x});

  wall = Object::cast_to<Spatial>(wall_scene_->instance());
  add_child(wall);
  wall->translate({0,0,start_z});
  wall->rotate_y(-Math_PI/2.0f);
  wall->set_scale({1, 1, start_x});

  wall = Object::cast_to<Spatial>(wall_scene_->instance());
  add_child(wall);
  wall->translate({start_x,0,0});
  wall->rotate_y(Math_PI);
  wall->set_scale({1, 1, start_z});

  wall = Object::cast_to<Spatial>(wall_scene_->instance());
  add_child(wall);
  wall->translate({-start_x,0,0});
  wall->rotate_y(-Math_PI);
  wall->set_scale({1, 1, start_z});

  auto spawn_height = 20.0f;
  Vector3 ball_start_pos = {start_x + 1, spawn_height, start_z+1};
  emit_signal("new_terrain_generated", ball_start_pos);
}