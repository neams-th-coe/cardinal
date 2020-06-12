[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
  [average_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = CoupledForce
    variable = temp
    v = heat_source
  []
[]

[BCs]
  [outside]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 300
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '0.1' # 20 W/mK -> 0.2 W/cmK
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 1000
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
  nl_rel_tol = 1e-5
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [./average_temp]
    type = NearestPointAverage
    variable = temp
    points = '0 0 0'
    execute_on = 'initial timestep_begin'
  []
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    app_type = OpenMCApp
    input_files = 'openmc.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [./heat_source_from_openmc]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    multi_app = openmc
    variable = heat_source
    source_variable = heat_source
    execute_on= 'initial timestep_begin'
  []
  [./average_temp_to_openmc]
    type = NearestPointReceiverTransfer
    direction = to_multiapp
    multi_app = openmc
    from_uo = average_temp
    to_uo = average_temp
  []
[]
