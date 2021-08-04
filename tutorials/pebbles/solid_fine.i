T_fluid = ${fparse 650.0 + 273.15}

[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = sphere_finer_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []
[]

[Variables]
  [temp]
    initial_condition = ${T_fluid}
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
  [surface]
    type = ConvectiveFluxFunction
    T_infinity = '${T_fluid}'
    coefficient = 1000.0
    variable = temp
    boundary = '1'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_values = '50.0'
    prop_names = 'thermal_conductivity'
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    app_type = OpenMCApp
    input_files = 'openmc_um.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [heat_source_from_openmc]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = openmc
    variable = heat_source
    source_variable = heat_source
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = heat_source_in_solid
  []
  [temp_to_openmc]
    type = MultiAppMeshFunctionTransfer
    direction = to_multiapp
    multi_app = openmc
    variable = temp
    source_variable = temp
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 2
  petsc_options_value = 'hypre boomeramg'
  dt = 1.0
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [heat_source_in_solid]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = 'transfer'
  []
[]
