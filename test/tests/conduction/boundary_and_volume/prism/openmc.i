# This is just a dummy application that stands in OpenMCs stead, by providing
# heat sources to BISON and nekRS.

[Mesh]
  type = FileMesh
  file = prism.exo

  # Refining the mesh gives better agreement with the MOOSE standalone case, but
  # to keep the gold file small, we dont refine here
  uniform_refine = 0
[]

[Variables]
  [dummy]
  []
[]

[AuxVariables]
  [source_bison] # This is calculated and sent to BISON
    block = '2'
  []
  [source_nek] # This is calculated and sent to nekRS
    block = '1'
  []
  [bison_temp] # This is received from BISON
    initial_condition = 500.0
  []
  [nek_temp] # This is received from nekRS
    # we set an initial condition because BISON will run first with this initial value
    initial_condition = 500.0
  []
  [bison_flux] # This is received from BISON
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[AuxKernels]
  [source1]
    type = ParsedAux
    variable = source_nek
    coupled_variables = 'nek_temp'
    expression = 'nek_temp+50'
    block = '1'
  []
  [source2]
    type = ParsedAux
    variable = source_bison
    coupled_variables = 'bison_temp'
    expression = '0.5*bison_temp+10'
    block = '2'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = dummy
    boundary = 'vol2_top'
    value = 1.0
  []
[]

[Postprocessors]
  [source_integral_nek]
    type = ElementIntegralVariablePostprocessor
    variable = source_nek
    block = '1'
  []
  [source_integral_bison]
    type = ElementIntegralVariablePostprocessor
    variable = source_bison
    execute_on = 'transfer'
    block = '2'
  []
  [flux_integral]
    type = Receiver
  []
  [max_T_int]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = max
    boundary = '2'
  []
  [min_T_int]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = min
    boundary = '2'
  []
  [max_T_nek]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = max
    block = '1'
  []
  [min_T_nek]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = min
    block = '1'
  []
  [avg_T_nek]
    type = ElementAverageValue
    variable = nek_temp
    block = '1'
  []
  [max_T_bison]
    type = NodalExtremeValue
    variable = bison_temp
    value_type = max
    block = '2'
  []
  [min_T_bison]
    type = NodalExtremeValue
    variable = bison_temp
    value_type = min
    block = '2'
  []
  [avg_T_bison]
    type = ElementAverageValue
    variable = bison_temp
    block = '2'
  []
  [pt_nek]
    type = PointValue
    point = '0 0 0'
    variable = nek_temp
  []
  [pt_bison]
    type = PointValue
    point = '0 0 -1'
    variable = bison_temp
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
  [bison]
    type = TransientMultiApp
    input_files = 'bison.i'
    execute_on = timestep_begin
    sub_cycling = true
  []
[]

[Transfers]
  [nek_temp_to_bison]
    type = MultiAppNearestNodeTransfer
    source_variable = nek_temp
    variable = nek_temp
    to_multi_app = bison

    # the nekRSMesh is a volume mesh, and to save on some of the data transfer, we can
    # just restrict this transfer between the two surfaces of interest because this
    # temperature is used in BISON only for a boundary condition
    source_boundary = '2'
    target_boundary = '2'
  []
  [source_to_bison]
    type = MultiAppNearestNodeTransfer
    source_variable = source_bison
    variable = source
    to_multi_app = bison
    from_postprocessors_to_be_preserved = source_integral_bison
    to_postprocessors_to_be_preserved = source_integral
    allow_skipped_adjustment = true
  []
  [bison_flux_to_openmc]
    type = MultiAppNearestNodeTransfer
    source_variable = flux
    from_multi_app = bison
    variable = bison_flux

    # the nekRSMesh is a volume mesh, and to save on some of the data transfer, we can
    # just restrict this transfer from the source surface of interest because this
    # flux is used in nekRS only for a boundary condition (at the time this test was
    # created, there was not an option to restrict to a target boundary for elementals)
    source_boundary = '2'
  []
  [bison_flux_integral_to_openmc]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    from_multi_app = bison
    reduction_type = 'average' # not used when only one sub-app
  []
  [bison_temp_to_openmc]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    variable = bison_temp
    from_multi_app = bison

    # IMPORTANT: this cannot be boundary restricted because we use this temperature to
    # compute a heat source for BISON
  []

  [bison_flux_to_nek]
    type = MultiAppNearestNodeTransfer
    source_variable = bison_flux
    variable = avg_flux
    to_multi_app = nek

    # the nekRSMesh is a volume mesh, and to save on some of the data transfer, we can
    # just restrict this transfer from the sourcesurface of interest because this
    # flux is used in nekRS only for a boundary condition (at the time this test was
    # created, there was not an option to restrict to a target boundary for elementals)
    source_boundary = '2'
  []
  [bison_flux_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
  [source_to_nek]
    type = MultiAppNearestNodeTransfer
    source_variable = source_nek
    variable = heat_source
    to_multi_app = nek
  []
  [source_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = source_integral
    from_postprocessor = source_integral_nek
    to_multi_app = nek
  []
  [nek_temp_to_openmc]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    variable = nek_temp
    from_multi_app = nek

    # IMPORTANT: this cannot be restricted to a boundary because we need it to
    # compute the heat source by OpenMC
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  num_steps = 30
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
  hide = 'source_bison source_nek dummy source_integral_nek source_integral_bison flux_integral'
[]
