extends ImmediateGeometry

func draw_line(start, end):
  get_transform().origin = start
  clear()
  begin(Mesh.PRIMITIVE_LINES)
  set_color(Color(1,1,1))
  # add vertex at 0 and shot vector (relative coordinates to ball)
  add_vertex(start)
  add_vertex(end)
  end()
