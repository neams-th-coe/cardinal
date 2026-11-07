[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [temperature]
      type = NekFieldVariable
      direction = from_nek
      mesh = 'mirror'
    []
    [temperature_stl]
      type = NekFieldVariable
      direction = from_nek
      mesh = 'stl'
      field = temperature
   []

    # in order to reuse some of the case files in this directory, we also have MOOSE set a boundary
    # condition for temperature so that when we compute a difference against moose_temp, the values on
    # the boundary are adequately considered in that difference
    [moose_temp]
      type = NekVolumetricData
      direction = to_nek
      usrwrk_slot = 1
    []
  []

  [Dimensionalize]
    L = 5.0
    dT = 50.0
    T = 46.0
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  stl = '../../nek_mesh/stl/small_sphere.stl'
  stl_multiplier = 0.25

  scaling = 5.0
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

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  # find the point at which the STL temperature is maximum
  [stl_x]
    type = ElementExtremeValue
    variable = x
    proxy_variable = temperature_stl
    block = 9999
  []
  [stl_y]
    type = ElementExtremeValue
    variable = y
    proxy_variable = temperature_stl
    block = 9999
  []
  [stl_z]
    type = ElementExtremeValue
    variable = z
    proxy_variable = temperature_stl
    block = 9999
  []
  [stl_max]
    type = ElementExtremeValue
    variable = temperature_stl
    block = 9999
  []

  # fetch the volume mesh mirror temperature at the same point; they should closely match
  [mirror]
    type = PointValue
    variable = temperature
    point = '0.4650204 0.4075408 -0.0406375'
  []
[]

[AuxVariables]
  [x]
  []
  [y]
  []
  [z]
  []
[]

[AuxKernels]
  [x]
    type = ParsedAux
    variable = x
    expression = 'x'
    use_xyzt = true
  []
  [y]
    type = ParsedAux
    variable = y
    expression = 'y'
    use_xyzt = true
  []
  [z]
    type = ParsedAux
    variable = z
    expression = 'z'
    use_xyzt = true
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
