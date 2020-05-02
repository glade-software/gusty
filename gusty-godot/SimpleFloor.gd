extends StaticBody

# Called when the node enters the scene tree for the first time.
func _ready():
  var shape = $CollisionShape.shape
  
  # width includes every point, but we want to loop over each "square" in the grid
  var w = shape.map_width
  var d = shape.map_depth
  
  # array of floats representing the height map
  var map_data = shape.map_data
  
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
  var start_x = - (w-1) * x_scale / 2
  var start_z = - (d-1) * z_scale / 2
  var vertex_pos = Vector3()
  
  var root_x : float
  var root_z : float
  var vertices : PoolVector3Array
  
  # looping over each square in the grid
  for i in range(d-1):
    for j in range(w-1):
      # corresponds to "top-left" vertex of the square in the grid
      root_x = start_x + j * x_scale
      root_z = start_z + i * z_scale

      vertices.resize(0)
      
      vertex_pos.x = root_x
      vertex_pos.z = root_z
      vertex_pos.y = map_data[w*i + j]
      vertices.push_back(vertex_pos)
      print("root vertex. index: " + str(w*i + j) + ", vertex: " + str(vertex_pos))
      
      vertex_pos.x = root_x + x_scale
      vertex_pos.z = root_z
      vertex_pos.y = map_data[w*i + j+1]
      vertices.push_back(vertex_pos)
      
      vertex_pos.x = root_x + x_scale
      vertex_pos.z = root_z + z_scale
      vertex_pos.y = map_data[w*(i+1) + j+1]
      vertices.push_back(vertex_pos)
      
      vertex_pos.x = root_x
      vertex_pos.z = root_z + z_scale
      vertex_pos.y = map_data[w*(i+1) + j]
      vertices.push_back(vertex_pos)
      
      st.add_triangle_fan(vertices)

  st.generate_normals()
  var generated_mesh = st.commit()
  $MeshInstance.mesh = generated_mesh
