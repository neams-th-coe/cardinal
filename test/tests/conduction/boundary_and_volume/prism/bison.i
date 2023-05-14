[Mesh]
  type = FileMesh
  file = bison.exo
  uniform_refine = 1
[]

[Variables]
  [temp]
    initial_condition = 500.0
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [source]
  []
  [nek_temp]
    # This initial value will be used in the first heat source sent to nekRS
    # because MOOSE runs first
    initial_condition = 500.0
  []
[]

[Functions]
  [vol2_top]
    type = ParsedFunction
    expression = '500.0+500*exp(y)'
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temp
    component = normal
    diffusivity = thermal_conductivity
    boundary = '2'
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
  [q]
    type = CoupledForce
    variable = temp
    v = source
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '5.0'
  []
[]

[BCs]
  [top]
    type = FunctionDirichletBC
    variable = temp
    boundary = 'vol2_top'
    function = 'vol2_top'
  []
  [interface]
    type = MatchedValueBC
    variable = temp
    boundary = '2'
    v = nek_temp
  []
[]

[Executioner]
  type = Transient
  num_steps = 15
  dt = 0.05
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12
[]

[Postprocessors]
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = '2'
  []
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = source
    execute_on = 'transfer'
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
  interval = 30
  hide = 'flux_integral source_integral'
[]
