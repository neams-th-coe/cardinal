[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  type = NekRSProblem
  casename = 'pipe'
  synchronization_interval = parent_app
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
  [nekbdry_icar]
    type = NekSideIntegral
    field = unity
    boundary = '2'
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [nekbdry_ar]
    type = NekSideIntegral
    field = unity
    boundary = '2'
    use_displaced_mesh = true
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

