[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gmsh]
    type = FileMeshGenerator
    file = 'block_oness.msh'
    dim = 3
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
    strain = SMALL
#    strain = FINITE
#    incremental = true
  []
[]

[Materials]
#  active = 'density stress elasticity_tensor'
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    shear_modulus = 5.0e5
    poissons_ratio = 0.4
    use_displaced_mesh = true
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1000.0
    use_displaced_mesh = true
  []
[]

[AuxVariables]
#  [nek_pressure]
#    order = SECOND
#  []
  [nek_tr_x]
    order = SECOND
  []
  [nek_tr_y]
    order = SECOND
  []
  [nek_tr_z]
    order = SECOND
  []
  [ramp]
    order = SECOND
  []
  [nek_tr_ramp_x]
    order = SECOND
  []
  [nek_tr_ramp_y]
    order = SECOND
  []
  [nek_tr_ramp_z]
    order = SECOND
  []
[]

[Functions]
  [amp1]
    type = ConstantFunction
    value = 0.0
  []
  [amp2]
    type = ParsedFunction
    expression = '(a2-a1)*(t-s2)/(s2-s1) + a2'
    symbol_names = 's1 s2 a1 a2'
    # symbol_values = '1e-3 1e-2 1e-9 0.5'
    symbol_values = '1e-3 1e-2 1e-9 1.0'
  []
  [amp3]
    type = ConstantFunction
    # value = 0.5
    value = 1.0
  []
  [amp_fn]
    type = PiecewiseFunction
    axis = t
    axis_coordinates = '1e-3 1e-2'
    functions = 'amp1 amp2 amp3'
  []
[]

[AuxKernels]
  [assign_amp]
    type = FunctionAux
    variable = ramp
    function = amp_fn
  []
  [ramp_tr_x]
    type = ParsedAux
    variable = nek_tr_ramp_x
    coupled_variables = 'ramp nek_tr_x'
    expression = '-ramp*nek_tr_x'
  []
  [ramp_tr_y]
    type = ParsedAux
    variable = nek_tr_ramp_y
    coupled_variables = 'ramp nek_tr_y'
    expression = '-ramp*nek_tr_y'
  []
  [ramp_tr_z]
    type = ParsedAux
    variable = nek_tr_ramp_z
    coupled_variables = 'ramp nek_tr_z'
    expression = '-ramp*nek_tr_z'
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
    use_displaced_mesh = true
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
    use_displaced_mesh = true
  []
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
    use_displaced_mesh = true
  []

  [sym_normal_bc]
    type = DirichletBC
    variable = disp_z
    boundary = sym
    value = 0
    use_displaced_mesh = true
  []

# traction boundary conditions
  [pressure_bc_left_x]
    type = CoupledVarNeumannBC
    variable = disp_x
    boundary = 'interface'
    v = nek_tr_ramp_x
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [pressure_bc_left_y]
    type = CoupledVarNeumannBC
    variable = disp_y
    boundary = 'interface'
    v = nek_tr_ramp_y
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
  [pressure_bc_left_z]
    type = CoupledVarNeumannBC
    variable = disp_z
    boundary = 'interface'
    v = nek_tr_ramp_z
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
#    execute_on = 'TIMESTEP_BEGIN'
#    execute_on = 'TIMESTEP_END' # this works
    sub_cycling = true
    use_displaced_mesh = true
    interpolate_transfers = true
  []
[]

[Transfers]
#   [synchronize]
#    type = MultiAppPostprocessorTransfer
#    to_postprocessor = transfer_in
#    from_postprocessor = synchronize
#    to_multi_app = nek
#  []
  [bdisp_x_to_nek]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = disp_x
    to_multi_app = nek
    variable = disp_x
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [bdisp_y_to_nek]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = disp_y
    to_multi_app = nek
    variable = disp_y
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [bdisp_z_to_nek]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = disp_z
    to_multi_app = nek
    variable = disp_z
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [traction_x_from_nek]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = traction_x
    from_multi_app = nek
    variable = nek_tr_x
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [traction_y_from_nek]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = traction_y
    from_multi_app = nek
    variable = nek_tr_y
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [traction_z_from_nek]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = traction_z
    from_multi_app = nek
    variable = nek_tr_z
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
[]

[Postprocessors]
#  [synchronize]
#    type = Receiver
#    default = 1
#  []
  [dofs]
    type = NumDOFs
  []

  [x_disp_pt]
    type = PointValue
    point = '0.45 0.15 -0.15'
    variable = disp_x
    use_displaced_mesh = true
  []
  [y_disp_pt]
    type = PointValue
    point = '0.45 0.15 -0.15'
    variable = disp_y
    use_displaced_mesh = true
  []
  [z_disp_pt]
    type = PointValue
    point = '0.45 0.15 -0.15'
    variable = disp_z
    use_displaced_mesh = true
  []

  [xdisp_time_change]
    type = ChangeOverTimePostprocessor
    postprocessor = x_disp_pt
    execute_on = 'timestep_end'
  []

  [max_tr_x]
    type = NodalExtremeValue
    variable = nek_tr_x
    value_type = max
  []
  [max_tr_y]
    type = NodalExtremeValue
    variable = nek_tr_y
    value_type = max
  []
  [max_tr_z]
    type = NodalExtremeValue
    variable = nek_tr_z
    value_type = max
  []
  [min_tr_x]
    type = NodalExtremeValue
    variable = nek_tr_x
    value_type = min
  []
  [min_tr_y]
    type = NodalExtremeValue
    variable = nek_tr_y
    value_type = min
  []
  [min_tr_z]
    type = NodalExtremeValue
    variable = nek_tr_z
    value_type = min
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
#  [./FDP]
#    type = FDP
#  [../]
[]

[Executioner]
  [Quadrature]
    type = GAUSS_LOBATTO
    order = FIFTH
  []
  type = Transient
  solve_type = 'PJFNK'
#  solve_type = 'NEWTON'
  end_time = 30.0
  dt = 1e-3
  dtmin = 1e-6
  abort_on_solve_fail = true

#  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_shift_type, -ksp_type'
#  petsc_options_value = 'asm 2 lu NONZERO gmres'

#  petsc_options = '-ksp_compute_eigenvalues -ksp_monitor_true_residual'

#  petsc_options_iname = '-pc_type'
#  petsc_options_value = 'hmg'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 100
  nl_max_its = 50

  nl_abs_tol = 1e-11
  l_tol = 1e-5
[]


[Outputs]
  exodus = true
[]

