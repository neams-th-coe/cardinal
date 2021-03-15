[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Problem]
  type = OpenMCProblem
  power = 0.15 # W
  centers = '0 0 0'

  # only needed for cell-type feedback, but included here for quick switching
  volumes = '1.279209e+01'

  tally_type = 'mesh'
  pebble_cell_level = 0
  mesh_template = 'sphere.e'
[]

[Executioner]
  type = Transient
  num_steps = 50
[]

[Postprocessors]
  [pebble_volume] # show the volume of the pebble for confirmation
    type = VolumePostprocessor
  []
[]

[Outputs]
  exodus = true
[]
