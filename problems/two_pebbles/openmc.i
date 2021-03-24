[Mesh]
  type = FileMesh
  file = two_sphere.e
[]

[Problem]
  type = OpenMCProblem
  power = 1000
  centers = '0 0 0 0 0 3.15'
  volumes = '13.986085'
  tally_type = 'mesh'
  mesh_template = 'sphere.e'
[]

[Executioner]
  type = Transient
  num_steps = 50
[]

[Postprocessors]
  [total_volume]
    type = VolumePostprocessor
  []
  [pebble_volume] # show the volume of each pebble for confirmation
    type = LinearCombinationPostprocessor
    pp_names = 'total_volume'
    pp_coefs = '${fparse 1.0/2.0}'
  []
[]

[Outputs]
  exodus = true

  [console]
    type = Console
    hide = 'total_volume'
  []
[]
