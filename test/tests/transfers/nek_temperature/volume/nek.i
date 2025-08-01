dTv = 10
Tv = 5

[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [temperature]
      type = NekFieldVariable
      direction = to_nek
      usrwrk_slot = 0
    []
    [nek_temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
  []

  [Dimensionalize]
    dT = ${dTv}
    T = ${Tv}
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[ICs]
  [tv]
    type = FunctionIC
    variable = temperature
    function = tv
  []
[]

[Functions]
  [tv] # temperature, dimensional
    type = ParsedFunction
    expression = 'sin(x)'
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

[AuxVariables]
  [d]
  []
[]

[AuxKernels]
  [d]
    type = ParsedAux
    variable = d
    expression = 'nek_temp - temperature'
    coupled_variables = 'nek_temp temperature'
  []
[]

[Postprocessors]
  [max_error]
    type = ElementExtremeValue
     variable = d
  []
[]

[Outputs]
  csv = true
[]
