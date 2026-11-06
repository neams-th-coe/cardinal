[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
[]

[Problem]
  type = NekRSProblem
  casename = "test"
  n_usrwrk_slots = 5

  synchronization_interval = parent_app

  [FieldTransfers]
    [source]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = 0
    []
    [temperature]
      type = NekFieldVariable
      direction = from_nek
    []
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
    min_dt = 1e-10
  []
[]

[Outputs]
  hide = 'source_integral'
[]
