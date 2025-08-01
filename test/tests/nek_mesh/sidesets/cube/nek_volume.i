[Problem]
  type = NekRSProblem
  casename = 'cube'
[]

[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  boundary = '1 2 3 4 5 6'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [area_side1_nek]
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [area_side1_moose]
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_side2_nek]
    type = NekSideIntegral
    field = unity
    boundary = '2'
  []
  [area_side2_moose]
    type = AreaPostprocessor
    boundary = '2'
  []
  [area_side3_nek]
    type = NekSideIntegral
    field = unity
    boundary = '3'
  []
  [area_side3_moose]
    type = AreaPostprocessor
    boundary = '3'
  []
  [area_side4_nek]
    type = NekSideIntegral
    field = unity
    boundary = '4'
  []
  [area_side4_moose]
    type = AreaPostprocessor
    boundary = '4'
  []
  [area_side5_nek]
    type = NekSideIntegral
    field = unity
    boundary = '5'
  []
  [area_side5_moose]
    type = AreaPostprocessor
    boundary = '5'
  []
  [area_side6_nek]
    type = NekSideIntegral
    field = unity
    boundary = '6'
  []
  [area_side6_moose]
    type = AreaPostprocessor
    boundary = '6'
  []
[]
