extends StaticBody

export(PackedScene) var wall_scene
signal reset_ball(ball_pos)

func new_mapdata_received(path):
  
  var height_map_shape = $CollisionShape.shape
  # hard coded for now - should have an import UI button in the test level
  
  # button should build height map, draw new surface, and reset player position
  
  var file = File.new()
  file.open(path, File.READ)
  
  # length of csv line array is map width
  # of lines read is map depth
  var map_width = file.get_csv_line().size()
  var map_depth = 0 # increment as file is read.
  
  var read_map_data = PoolRealArray()
  
  file.seek(0)
  while not file.eof_reached():
    var csv_line = file.get_csv_line()
    
    # excel adds a blank line at bottom of file, don't want to count that toward map_depth
    if csv_line.size() != map_width:
      break
    for num in csv_line:
      if num.is_valid_float():
        read_map_data.push_back(num.to_float())
    map_depth = map_depth + 1
  file.close()
  
  # Quit if loaded matrix is not a perfect rectangke
  if read_map_data.size() != (map_width*map_depth):
    print("error reading height map file " + path)
    print("width: " + str(map_width) + ", height: " + str(map_depth))
    return
  
  # load in read height map to collisionshape
  height_map_shape.map_width = map_width
  height_map_shape.map_depth = map_depth
  height_map_shape.map_data = read_map_data
  
  var material = SpatialMaterial.new()
  # green
  material.albedo_color = Color(0.4, 1.0, 0.1, 1.0)
  
  var st = SurfaceTool.new()
  st.begin(Mesh.PRIMITIVE_TRIANGLES)
  st.set_material(material)
  st.add_uv(Vector2(0, 0)) # not sure what this does
  
  #units to step for each "square/rect" - will probably stay 1
  var x_scale = $CollisionShape.scale.x
  var z_scale = $CollisionShape.scale.z
  
  # top left of entire height map
  var start_x = - (map_width-1) * x_scale / 2
  var start_z = - (map_depth-1) * z_scale / 2
  var vertex_pos = Vector3()
  
  var root_x : float
  var root_z : float
  var vertices : PoolVector3Array
  
  # looping over each square in the grid
  for i in range(map_depth-1):
    for j in range(map_width-1):
      # corresponds to "top-left" vertex of the square in the grid
      root_x = start_x + j * x_scale
      root_z = start_z + i * z_scale

      vertices.resize(0)
      vertex_pos.x = root_x
      vertex_pos.z = root_z
      vertex_pos.y = read_map_data[map_width*i + j]
      vertices.push_back(vertex_pos)
      
      vertex_pos.x = root_x + x_scale
      vertex_pos.z = root_z
      vertex_pos.y = read_map_data[map_width*i + j+1]
      vertices.push_back(vertex_pos)
      
      vertex_pos.x = root_x + x_scale
      vertex_pos.z = root_z + z_scale
      vertex_pos.y = read_map_data[map_width*(i+1) + j+1]
      vertices.push_back(vertex_pos)
      
      vertex_pos.x = root_x
      vertex_pos.z = root_z + z_scale
      vertex_pos.y = read_map_data[map_width*(i+1) + j]
      vertices.push_back(vertex_pos)
      
      # find steepest diagonal
      var slope02 = abs(vertices[0].y - vertices[2].y)
      var slope13 = abs(vertices[1].y - vertices[3].y)
      # this is to ensure consistent, more "concave" like slopes
      if slope02 > slope13:
        # shift elements by one to change draw order
        vertices.insert(0, vertices[3])
        vertices.remove(4)
      
      st.add_triangle_fan(vertices)

  st.generate_normals()
  var generated_mesh = st.commit()
  $MeshInstance.mesh = generated_mesh
  
  # instance walls, set them to correct positions
  # get x z position right
  # rotate about y axis
  # stretch in its own z axis
  #changes the wall, so shouldn't have to adjust y pos
  
  # First wall, make centre max z wall
  var wall = wall_scene.instance()
  add_child(wall)
  wall.translation.z = start_z
  wall.rotation.y = PI/2
  wall.scale.z = start_x
  wall = wall_scene.instance()
  add_child(wall)
  wall.translation.z = -start_z
  wall.rotation.y = -PI/2
  wall.scale.z = start_x
  wall = wall_scene.instance()
  add_child(wall)
  wall.translation.x = start_x
  wall.rotation.y = PI
  wall.scale.z = start_z
  
  wall = wall_scene.instance()
  add_child(wall)
  wall.translation.x = -start_x
  wall.rotation.y = -PI
  wall.scale.z = start_z
  
  # where should we spawn ball?
  var ball_pos = Vector3(start_x + 1, 1, start_z + 1)
  
  emit_signal("reset_ball", ball_pos)
