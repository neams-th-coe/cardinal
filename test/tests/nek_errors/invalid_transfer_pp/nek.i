[Problem]
  type = NekRSProblem
  synchronization_interval = parent_app
  casename = 'cube'

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '4'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
