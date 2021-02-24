[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Problem]
  type = OpenMCProblem
  power = 0.15 # W
  centers = '0 0 0'
  volumes = '14.137166941154067'
  tally_type = 'mesh'
  pebble_cell_level = 0
  mesh_template = 'sphere.e'
[]

[MultiApps]
  [bison]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'moose.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [./heat_source_from_openmc]
    type = MultiAppCopyTransfer
    direction = to_multiapp
    multi_app = bison
    variable = heat_source

    # this variable is defined in the OpenMCProblem, so it doesnt appear in this file
    source_variable = heat_source
  []
  [./average_temp_to_openmc]
    type = NearestPointReceiverTransfer
    direction = from_multiapp
    multi_app = bison
    from_uo = average_temp

    # this userobject is defined in the OpenMCProblem, so it doesnt appear in this file
    to_uo = average_temp
  []
[]

[AuxVariables]
  [average_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 50
[]

[Outputs]
  exodus = true
[]
