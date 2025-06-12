[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [Dimensionalize]
    U = 2.0
  []

  [FieldTransfers]
    [velocity]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_z_squared]
      type = NekFieldVariable
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

# dimensional form
[Functions]
  [vx]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [vy]
    type = ParsedFunction
    expression = '(y+1)*2'
  []
  [vz]
    type = ParsedFunction
    expression = 'exp(x*y*z)*2'
  []
  [v]
    type = ParsedFunction
    expression = 'sqrt(vx*vx+vy*vy+vz*vz)'
    symbol_names = 'vx vy vz'
    symbol_values = 'vx vy vz'
  []
  [vz_2]
    type = ParsedFunction
    expression = 'vz*vz'
    symbol_names = 'vz'
    symbol_values = 'vz'
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
  [v_error]
    type = ElementL1Error
     variable = velocity
     function = v
  []
  [vz_2_error]
    type = ElementL1Error
     variable = velocity_z_squared
     function = vz_2
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
