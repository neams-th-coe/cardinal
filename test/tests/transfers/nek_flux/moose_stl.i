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
    diffusivity = 2.0
    boundary = 0
    component = normal
  []
[]

[Postprocessors]
  [flux_integral_1]
    type = SideDiffusiveFluxIntegral
    variable = moose_temp
    boundary = 0
    diffusivity = 2.0
  []
  [flux_integral_2]
    type = SideIntegralVariablePostprocessor
    variable = moose_flux
    boundary = 0
  []
  [area]
    type = AreaPostprocessor
    boundary = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
  exodus = true
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'stl.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [moose_flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = nek
    source_variable = moose_flux
    variable = moose_flux
    from_boundaries = '0'
    to_blocks = '9999'
  []
[]
