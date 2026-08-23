[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  type = NekRSProblem
  casename = 'nekbox'
  n_usrwrk_slots = 4

  [FieldTransfers]
    [heat_source]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = source_integral
    []
    [disp]
      type = NekMeshDeformation
      usrwrk_slot = '1 2 3'
      direction = to_nek
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
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
  [nekbdry_icar1]
    type = AreaPostprocessor
    boundary = '1'
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [nekbdry_ar1]
    type = AreaPostprocessor
    boundary = '1'
    use_displaced_mesh = true
  []
  [nekbdry_ar2]
    type = AreaPostprocessor
    boundary = '2'
    use_displaced_mesh = true
  []
  [nekbdry_ar3]
    type = AreaPostprocessor
    boundary = '3'
    use_displaced_mesh = true
  []
  [nekbdry_ar4]
    type = AreaPostprocessor
    boundary = '4'
    use_displaced_mesh = true
  []
  [nekbdry_ar5]
    type = AreaPostprocessor
    boundary = '5'
    use_displaced_mesh = true
  []
  [nekbdry_ar6]
    type = AreaPostprocessor
    boundary = '6'
    use_displaced_mesh = true
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'

  # uncomment the temp_ansol to see that the solution matches very well
  hide = 'source_integral heat_source'
[]

