!include mesh_tallies/openmc.i

[Functions]
  [turn_on_amr]
    type = ParsedFunction
    expression = '((t > 1.0) & (t < 3.0))'
  []
  [turn_on_relaxation]
    type = ParsedFunction
    expression = '((t >= 2.0) & (t < 4.0)) | (t > 5.0)'
  []
[]

[Controls]
  [control_amr]
    type = BoolFunctionControl
    function = 'turn_on_amr'
    parameter = 'amr/*/enable'
    # Execute before we run OpenMC
    execute_on = 'TIMESTEP_BEGIN'
  []
  [control_relaxation]
    type = BoolFunctionControl
    function = 'turn_on_relaxation'
    parameter = '*/*/relaxation_on'
    # Execute before we run OpenMC
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Adaptivity]
  marker = uniform

  [Markers/uniform]
    type = UniformMarker
    mark = 'REFINE'
  []

  control_tags = 'amr'
[]

[Problem]
  relaxation := 'constant'
  xml_directory = './mesh_tallies'
[]

[Executioner]
  num_steps := 6
[]

[Outputs]
  exodus := false
  execute_on = 'TIMESTEP_END'
[]
