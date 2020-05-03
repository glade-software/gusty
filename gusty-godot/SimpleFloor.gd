extends StaticBody

func _ready():
  var height_map_shape = $CollisionShape.shape
  
  # hard coded for now - should have an import UI button in the test level
  
  # button should build height map, draw new surface, and reset player position
  
  var filename = "res://level-data/test.csv"
  var file = File.new()
  file.open(filename, File.READ)
  
  # length of csv line array is map width
  # of lines read is map depth
  var map_width = file.get_csv_line().size()
  var map_depth = 0 # increment as file is read.
  
  var read_map_data = PoolRealArray()
  
  file.seek(0)
  
  # no error checking, assume the csv is valid and a rectangular matrix of vals
  while not file.eof_reached():
    var csv_line = file.get_csv_line()
    
    # excel adds a blank line at bottom of file, don't want to count that toward map_depth
    if csv_line.size() != map_width:
      break
    for num in csv_line:
      # change string to float?
      read_map_data.push_back(num.to_float())
      print("adding num: " + num)
    map_depth = map_depth + 1
    
  if read_map_data.size() != (map_width*map_depth):
    print("error reading height map file " + filename)
    print("width: " + str(map_width) + ", height: " + str(map_depth))
  file.close()
  
  # load in read height map to collisionshape
  height_map_shape.map_width = map_width
  height_map_shape.map_depth = map_depth
  height_map_shape.map_data = read_map_data
  
  var material = SpatialMaterial.new()
  # green
  material.albedo_color = Color(0.0, 1.0, 0.0, 1.0)
  
  var st = SurfaceTool.new()
  st.begin(Mesh.PRIMITIVE_TRIANGLES)
  st.set_material(material)
  st.add_uv(Vector2(0, 0))
  
  #units to step for each "square/rect" - should probably stay 1
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
