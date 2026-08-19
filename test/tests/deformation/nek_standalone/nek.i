[Mesh]
  type = NekRSMesh
  volume = true
[]

[Functions]
  [y_coordinate]
    type = ParsedFunction
    expression = 'y'
  []
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

  [y_integral]
    type = NekVolumeIntegral
    function = y_coordinate
  []

  [host_coordinates_updated]
    type = ParsedPostprocessor
    expression = 'if(abs(y_integral / volume + 0.25) > 1e-3, 1, 0)'
    pp_names = 'y_integral volume'
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
  hide = 'y_integral'
[]
