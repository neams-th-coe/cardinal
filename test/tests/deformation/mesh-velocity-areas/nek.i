[Mesh]
  type = NekRSMesh
  order = SECOND
  boundary = '2'
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh =true
[]

[Problem]
  type = NekRSProblem
  has_heat_source = false
  casename = 'pipe'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []

  [Quadrature]
    type = GAUSS_LOBATTO
    order = SECOND
  []
[]

[Postprocessors]
  [nek_area_initial]
    type = NekSideIntegral
    field = unity
    boundary = '2'
    execute_on = initial
  []
  [nek_area]
    type = NekSideIntegral
    field = unity
    boundary = '2'
  []
  [nek_diff]
    type = DifferencePostprocessor
    value1 = nek_area
    value2 = nek_area_initial
  []
  [moose_area_initial]
    type = VolumePostprocessor # for a side-only mesh, this is area
    execute_on = initial
    use_displaced_mesh = true
  []
  [moose_area]
    type = VolumePostprocessor
    use_displaced_mesh = true
  []
  [moose_diff]
    type = DifferencePostprocessor
    value1 = moose_area
    value2 = moose_area_initial
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
  hide = 'flux_integral nek_area_initial nek_area moose_area_initial moose_area'
[]
