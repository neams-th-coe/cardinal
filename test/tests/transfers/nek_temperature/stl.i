[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [temperature]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
  stl = '../small_sphere.stl'
[]

[AuxVariables]
  [moose_temp]
  []
  [d]
  []
[]

[Functions]
  [moose_temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
[]

[AuxKernels]
  [moose_temp]
    type = FunctionAux
    variable = moose_temp
    function = moose_temp
    block = 9999
  []
  [d]
    type = ParsedAux
    variable = d
    expression = 'moose_temp - temperature'
    coupled_variables = 'moose_temp temperature'
    block = 9999
  []
[]

[Postprocessors]

  # we do not expect these to be perfectly zero, because we are interpolating the moose_temp
  # function (defined in the udf file) onto the STL points
  [max_error]
    type = ElementExtremeValue
    variable = d
  []
  [min_error]
    type = ElementExtremeValue
    variable = d
    value_type = min
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
