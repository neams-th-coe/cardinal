[Mesh]
  type = NekRSMesh
  order = SECOND
  boundary = '3'
#  volume = true
#  exact = false
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Problem]
  type = NekRSProblem
  casename = 'richter'
  output = 'traction'
  has_heat_source = false
#  synchronization_interval = parent_app
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [nekbdry_icar]
    type = NekSideIntegral
    field = unity
    boundary = '3'
    execute_on = INITIAL
  []
  [nekbdry_ar]
    type = NekSideIntegral
    field = unity
    boundary = '3'
  []
  [min_tr_x]
    type = NekSideExtremeValue
    field = traction_x
    boundary = '3'
    value_type = min
  []
  [min_tr_y]
    type = NekSideExtremeValue
    field = traction_y
    boundary = '3'
    value_type = min
  []
  [min_tr_z]
    type = NekSideExtremeValue
    field = traction_z
    boundary = '3'
    value_type = min
  []
  [max_tr_x]
    type = NekSideExtremeValue
    field = traction_x
    boundary = '3'
    value_type = max
  []
  [max_tr_y]
    type = NekSideExtremeValue
    field = traction_y
    boundary = '3'
    value_type = max
  []
  [max_tr_z]
    type = NekSideExtremeValue
    field = traction_z
    boundary = '3'
    value_type = max
  []
  [nek_drag]
    type = NekSideAverage
    field = pressure
    boundary = '3'
  []
  [nek_drag_psf]
    type = NekPressureSurfaceForce
    boundary = '3'
    direction = '1 0 0'
    mesh = 'fluid'
  []
[]

[Outputs]
  exodus = true
[]

