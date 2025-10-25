[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 2
  usrwrk_output = '0 1'
  usrwrk_output_prefix = 'one two'

  [FieldTransfers]
    [src1]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = 0
      initial_source_integral = 10
    []
    [src2]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = 1
      initial_source_integral = 15
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[ICs]
  [src1]
    type = ConstantIC
    variable = src1
    value = 1
  []
  [src2]
    type = ConstantIC
    variable = src2
    value = 1
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [usrwrk1]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 0
    boundary = '1'
  []
  [usrwrk2]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 1
    boundary = '1'
  []
  [area]
    type = NekSideIntegral
    boundary = '1'
    field = unity
  []
  [volume]
    type = NekVolumeIntegral
    field = unity
  []
[]

[Outputs]
  csv = true
[]
