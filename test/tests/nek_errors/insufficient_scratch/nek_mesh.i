[Mesh]
  type = NekRSMesh
  volume = true
  displacements = 'dx dy dz'
[]

[AuxVariables]
  [dx]
  []
  [dy]
  []
  [dz]
  []
[]

[Problem]
  type = NekRSProblem
  casename = 'brick2'
  has_heat_source = false
  n_usrwrk_slots = 1
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
