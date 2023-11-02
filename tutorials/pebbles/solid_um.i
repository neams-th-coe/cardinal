T_fluid = ${fparse 650.0 + 273.15}

[Mesh]
  [pebble]
    type = SphereMeshGenerator
    nr = 4
    radius = 0.015
  []
  [rotate] # we can rotate the pebble, MOOSEs mesh transfers still handle the transfer properly
    type = TransformGenerator
    input = pebble
    transform = rotate
    vector_value = '0.5 0.2 0.2'
  []
  [repeat]
    type = CombinerGenerator
    inputs = rotate
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

[Functions]
  [T_fluid]
    type = ParsedFunction
    expression = '${T_fluid}+z*1000'
  []
[]

[BCs]
  [surface]
    type = ConvectiveFluxFunction
    T_infinity = T_fluid
    coefficient = 1000.0
    variable = temp
    boundary = '0'
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
    app_type = CardinalApp
    input_files = 'openmc_um.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [heat_source_from_openmc]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = openmc
    variable = heat_source
    source_variable = heat_source
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = source_integral
  []
  [temp_to_openmc]
    type = MultiAppShapeEvaluationTransfer
    to_multi_app = openmc
    variable = temp
    source_variable = temp
  []
[]

[Postprocessors]
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = transfer
  []
  [max_T]
    type = NodalExtremeValue
    variable = temp
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 3
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
[]
