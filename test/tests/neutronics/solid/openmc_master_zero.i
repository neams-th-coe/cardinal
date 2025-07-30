[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [multiple]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
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
  [average_temp] # unused in the coupling; this is just present to avoid a re-gold
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
    type = FunctionDirichletBC
    variable = temp
    boundary = '1'
    function = 'axial'
  []
[]

zmin = -1.5
zmax = 9.5

m=${fparse 500/(zmax-zmin)}
b=${fparse 300-500/(zmax-zmin)*zmin}

[Functions]
  [axial]
    type = ParsedFunction
    expression = '${m}*z+${b}'
  []
[]

[AuxKernels]
  [average_temp]
    variable = average_temp
    type = SpatialUserObjectAux
    user_object = average_temp
    execute_on = 'initial timestep_end'
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '0.2' # 20 W/mK -> 0.2 W/cmK
    prop_names = 'thermal_conductivity'
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
  execute_on = 'final'
  hide = 'power_from_openmc'
[]

[UserObjects]
  [average_temp]
    type = NearestPointAverage
    variable = temp
    points_file = pebble_centers_rescaled.txt
    execute_on = 'initial timestep_end'
  []
[]

[Postprocessors]
  [power_from_openmc]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [max_T]
    type = ElementExtremeValue
    variable = temp
    execute_on = 'initial timestep_end'
    value_type = max
  []
  [min_T]
    type = ElementExtremeValue
    variable = temp
    execute_on = 'initial timestep_end'
    value_type = min
  []
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    input_files = 'openmc_zero.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [heat_source_from_openmc]
    type = MultiAppCopyTransfer
    from_multi_app = openmc
    variable = heat_source
    source_variable = heat_source
  []
  [average_temp_to_openmc]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = openmc
    source_variable = temp
    variable = temp
  []
[]
