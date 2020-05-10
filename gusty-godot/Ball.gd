extends RigidBody

export(NodePath) var line_path

var line

# multiply by the line visual aid so it's not so big
const LINE_SCALE = 0.2

var winding_up_shot = true

var total_shot_vector = Vector3()
  
func _input(event):
  # currently mapped to space
  if event.is_action_pressed("hit_ball"):
    print("vec for shot" + str(total_shot_vector))
    apply_central_impulse(total_shot_vector)
    clear_vector()
    # get_node(line_path).draw_line(translation, translation + total_shot_vector)

func _process(delta):
  # right now always true, but what we can do if have it become true once the 
  # ball has slowed down enough (or becomes still)
  if winding_up_shot:
    get_node(line_path).draw_line(translation, translation + total_shot_vector)

# connect ui signal to this (in future, this function will be trigger by networking event)
func add_vector(vector):
  total_shot_vector = total_shot_vector + vector
  print("vector addedm total is" + str(total_shot_vector))
  get_node(line_path).draw_line(translation, translation + total_shot_vector)

func clear_vector():
  total_shot_vector = Vector3()

func reset(position):
  # Just don't bother resetting the shot vector / speed yet
  translation = position
