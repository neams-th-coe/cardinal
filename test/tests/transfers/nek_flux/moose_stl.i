[Mesh]
  [sphere]
    type = SphereMeshGenerator
    nr = 4
    radius = 0.5
  []
  [translate]
    type = TransformGenerator
    input = sphere
    transform = translate
    vector_value = '0.5 0.5 0.5'
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [moose_temp]
  []
  [moose_flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [moose_temp]
    type = FunctionIC
    variable = moose_temp
    function = moose_temp
  []
[]

[Functions]
  [moose_temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
[]

[AuxKernels]
  [moose_flux]
    type = DiffusionFluxAux
    variable = moose_flux
    diffusion_variable = moose_temp
    diffusivity = 1.0
    boundary = 0
    component = normal
  []
[]

[Postprocessors]
  [flux_integral_1]
    type = SideDiffusiveFluxIntegral
    variable = moose_temp
    boundary = 0
    diffusivity = 1.0
  []
  [flux_integral_2]
    type = SideIntegralVariablePostprocessor
    variable = moose_flux
    boundary = 0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  exodus = true
[]
