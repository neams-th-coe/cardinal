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

  [min_y]
    type = NekMeshInfoPostprocessor
    test_type = min_node_y
  []

  [host_coordinates_updated]
    type = ParsedPostprocessor
    expression = 'if(min_y > -0.99, 1, 0)'
    pp_names = 'min_y'
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
  hide = 'min_y'
[]
