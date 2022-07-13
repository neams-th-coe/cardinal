[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
  moving_mesh = true
[]

[Problem]
  type = NekRSProblem
  casename = 'nekbox'
[]

[AuxVariables]
  [dummy]
  []
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
  []
[]

# This AuxVariable and AuxKernel is only here to get the postprocessors
# to evaluate correctly. This can be deleted after MOOSE issue #17534 is fixed.
[AuxKernels]
  [dummy]
    type = ConstantAux
    variable = dummy
    value = 0.0
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
    displacements = 'disp_x disp_y disp_z'
  []
  [nekbdry_ar1]
    type = AreaPostprocessor
    boundary = '1'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [nekbdry_ar2]
    type = AreaPostprocessor
    boundary = '2'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [nekbdry_ar3]
    type = AreaPostprocessor
    boundary = '3'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [nekbdry_ar4]
    type = AreaPostprocessor
    boundary = '4'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [nekbdry_ar5]
    type = AreaPostprocessor
    boundary = '5'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [nekbdry_ar6]
    type = AreaPostprocessor
    boundary = '6'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Outputs]
  exodus = false
  csv = true
  execute_on = 'final'

  # uncomment the temp_ansol to see that the solution matches very well
  hide = 'source_integral heat_source dummy'
[]

