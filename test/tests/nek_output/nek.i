[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [FieldTransfers]
    [scalar01]
      type = NekFieldVariable
      direction = from_nek
    []
    [scalar02]
      type = NekFieldVariable
      direction = from_nek
    []
    [scalar03]
      type = NekFieldVariable
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Functions]
  [s01]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [s02]
    type = ParsedFunction
    expression = 'y+1'
  []
  [s03]
    type = ParsedFunction
    expression = 'exp(x*y*z)'
  []
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
  [s01_error]
    type = ElementL1Error
     variable = scalar01
     function = s01
  []
  [s02_error]
    type = ElementL1Error
     variable = scalar02
     function = s02
  []
  [s03_error]
    type = ElementL1Error
     variable = scalar03
     function = s03
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
