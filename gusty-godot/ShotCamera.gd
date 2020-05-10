extends Camera


export(NodePath) var target
export var dist_behind = 6.0
export var dist_above = 2.5
export var down_tilt = 0.0 # degrees
export var speed = 2.0
# Z IS BACKWARDS FROM WHERE I THOUGHT - CAMERA FACES TOWARD -z
# camera front (self -z)
# camera right (self +x)
# camera up (self +y)

var target_node: Spatial
var offset: Vector3

# Called when the node enters the scene tree for the first time.
func _ready():
  offset = Vector3(0, dist_above, -dist_behind)
  
  target_node = get_node(target)
  translation = target_node.translation + offset
  rotation.y = PI
  
  # add rotation to x axis
#  rotation.x -= down_tilt

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
  translation = translation.linear_interpolate(target_node.translation + offset, speed*delta)
  
  rotation_degrees.x = -down_tilt
#  translation = target_node.translation + offset
