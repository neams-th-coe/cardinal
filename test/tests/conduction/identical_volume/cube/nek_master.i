# The master app performs a dummy solve - we are just testing the ability for nekRS
# to send temperature to BISON, which performs some operation on it to return a heat
# source to nekRS. Here, that operation is to multiply temperature by 7.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 7
  ny = 7
  nz = 7
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [source]
  []
  [nek_temp]
    initial_condition = 500.0
  []
[]

[AuxKernels]
  [source]
    type = ParsedAux
    variable = source
    expression = 'nek_temp*7'
    coupled_variables = 'nek_temp'
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 15
  dt = 0.1
  nl_abs_tol = 1e-8
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
    greedy_search = true
  []
  [source]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = source
    to_multi_app = nek
    variable = heat_source
    greedy_search = true
  []
  [source_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = source_integral
    from_postprocessor = source_integral
    to_multi_app = nek
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = source
  []
[]

[Outputs]
  print_linear_residuals = false

  [exo]
    type = Exodus
    execute_on = 'final'
  []

  hide = 'source_integral source'
[]
