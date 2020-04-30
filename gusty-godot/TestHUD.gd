extends CanvasLayer

const current_vec_zero_str = "(0.0, 0.0, 0.0)"

#total vec duplicated in ball script
var total_vec = Vector3()

var magnitude = 0.0

signal new_vector(impulse_vec)

func _ready():
  $LabelCurrentVec.text = current_vec_zero_str


func _input(event):
  if event.is_action_pressed("add_vector"):
    var vec_to_send = $spnMag.value * Vector3($spnX.value, $spnY.value, $spnZ.value).normalized()
    print("vec to send: " + str(vec_to_send))
    total_vec = total_vec + vec_to_send
    $LabelCurrentVec.text = str(total_vec)
    emit_signal("new_vector", vec_to_send)
  elif event.is_action_pressed("hit_ball"):
    clear_vec() 

func clear_vec():
  total_vec = Vector3()
  $LabelCurrentVec.text = current_vec_zero_str
