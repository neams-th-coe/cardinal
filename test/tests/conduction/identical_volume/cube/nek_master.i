# The master app performs a dummy solve - we are just testing the ability for nekRS
# to send temperature to BISON, which performs some operation on it to return a heat
# source to nekRS. Here, that operation is to multiply temperature by 7.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 40
  nz = 40
[]

[Variables]
  [dummy]
  []
[]

[AuxVariables]
  [source]
  []
  [nek_temp]
    # This initial value will be used in the first heat source sent to nekRS
    # because MOOSE runs first
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

[Kernels]
  [diffusion]
    type = Diffusion
    variable = dummy
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = dummy
    value = 1.0
    boundary = 'right'
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
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [source]
    type = MultiAppNearestNodeTransfer
    source_variable = source
    to_multi_app = nek
    variable = heat_source
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
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
[]
