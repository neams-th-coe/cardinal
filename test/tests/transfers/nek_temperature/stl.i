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

  [Dimensionalize]
    dT = 50.0
    T = 100.0
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
  stl = '../small_sphere.stl'
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
