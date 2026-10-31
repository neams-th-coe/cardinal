[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [temperature]
      type = NekFieldVariable
      direction = from_nek
      mesh = 'stl mirror'
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '2'
  stl = '../../nek_mesh/stl/small_sphere.stl'
  stl_multiplier = 0.25
[]

[AuxVariables]
  [moose_temp]
  []
  [d_t]
  []
[]

[Functions]
  [moose_temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
[]

[ICs]
  [moose_temp]
    type = FunctionIC
    variable = moose_temp
    function = moose_temp
  []
[]

[AuxKernels]
  [d_t]
    type = ParsedAux
    variable = d_t
    expression = '(moose_temp - temperature) / moose_temp'
    coupled_variables = 'moose_temp temperature'
  []
[]

[Postprocessors]
  # we do not expect these to be perfectly zero, because we are interpolating the
  # function (defined in the udf file) onto the STL points
  [max_error_t_0]
    type = ElementExtremeValue
    variable = d_t
    block = 0
  []
  [min_error_t_0]
    type = ElementExtremeValue
    variable = d_t
    value_type = min
    block = 0
  []
  [max_error_t_9999]
    type = ElementExtremeValue
    variable = d_t
    block = 9999
  []
  [min_error_t_9999]
    type = ElementExtremeValue
    variable = d_t
    value_type = min
    block = 9999
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
