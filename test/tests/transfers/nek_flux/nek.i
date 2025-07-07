dTv = 10
Tv = 5

[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = from_nek
    []
  []

  [Dimensionalize]
    dT = ${dTv}
    T = ${Tv}
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []

  [Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  []
[]

[Postprocessors]
  [read_flux]
    type = ElementIntegralVariablePostprocessor
    variable = flux
  []
  [analytic_flux]
    type = ElementIntegralVariablePostprocessor
    variable = moose_flux
  []
  [max_error]
    type = ElementExtremeValue
    variable = d
  []
[]

[AuxVariables]
  [moose_flux]
  []
  [d]
  []
[]

[AuxKernels]
  [moose_flux]
    type = FunctionAux
    variable = moose_flux
    function = f
  []
  [d]
    type = ParsedAux
    variable = d
    expression = 'flux - moose_flux'
    coupled_variables = 'flux moose_flux'
  []
[]

[Functions]
  [f] # dimensionalized
    type = ParsedFunction
    expression = '2*(cos(y)+x*z) * ${dTv}'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = '2'
  []
[]

[Outputs]
  csv = true
[]
