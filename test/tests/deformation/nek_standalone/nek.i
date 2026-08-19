[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'mv_cyl'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [volume]
    type = NekVolumeIntegral
    field = unity
  []
  [area]
    type = NekSideIntegral
    field = unity
    boundary = '1 2 3'
  []

  # these will not reflect the changing mesh, because we do not copy displacements
  # from NekRS to MOOSE
  [volume_moose]
    type = VolumePostprocessor
  []
  [area_moose]
    type = AreaPostprocessor
    boundary = '1 2 3'
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
