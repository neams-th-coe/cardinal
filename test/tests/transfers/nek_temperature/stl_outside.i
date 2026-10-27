[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 3

  [FieldTransfers]
    [scalar01]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  stl = '../../nek_mesh/stl/small_sphere.stl'
[]

[Postprocessors]
  [max_t]
    type = ElementExtremeValue
    variable = scalar01
    block = 9999
  []
  [min_t]
    type = ElementExtremeValue
    variable = scalar01
    value_type = min
    block = 9999
  []
  [max_nek_t]
    type = NekVolumeExtremeValue
    field = scalar01
    block = 9999
  []
  [min_nek_t]
    type = NekVolumeExtremeValue
    field = scalar01
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
