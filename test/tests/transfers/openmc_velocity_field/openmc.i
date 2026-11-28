[Mesh]
  [velocity_mesh]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 100.0
  []
[]

[Functions]
  [vel_x_ic_function]
    type = ParsedFunction
    expression = "1.0 + x + 0.1 * y + 0.01 * z"
  []
  [vel_y_ic_function]
    type = ParsedFunction
    expression = "2.0 + x + 0.1 * y + 0.01 * z"
  []
  [vel_z_ic_function]
    type = ParsedFunction
    expression = "3.0 + x + 0.1 * y + 0.01 * z"
  []
[]

[ICs]
  [vel_x]
    type = FunctionIC
    variable = velocity_x
    function = vel_x_ic_function
  []
  [vel_y]
    type = FunctionIC
    variable = velocity_y
    function = vel_y_ic_function
  []
  [vel_z]
    type = FunctionIC
    variable = velocity_z
    function = vel_z_ic_function
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  velocity_field_transfer = true

  scaling = 100.0

  velocity_blocks = '0'
  cell_level = 0

  power = 1.0

  [Tallies]
    [heat_source]
      type = MeshTally
      output = unrelaxed_tally_std_dev
    []
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [velocity_x_to_openmc]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = nek
    variable = velocity_x
    source_variable = velocity_x
  []
  [velocity_y_to_openmc]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = nek
    variable = velocity_y
    source_variable = velocity_y
  []
  [velocity_z_to_openmc]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = nek
    variable = velocity_z
    source_variable = velocity_z
  []
  [power_to_nek]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = nek
    source_variable = kappa_fission
    variable = source
  []
  [power_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = source_integral
    from_postprocessor = power
    to_multi_app = nek
  []
  [synchronize_in]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronize
    to_multi_app = nek
  []
[]

[Postprocessors]
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [synchronize]
    type = Receiver
    default = 1.0
  []
[]

[VectorPostprocessors]
  [openmc_vel_field]
    type = OpenMCVelocityField
    execute_on = "timestep_end"
    sort_by = "cell_id"
  []
[]

[Executioner]
  type = Transient
  dt = 2.5e-3
  num_steps = 1
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'synchronize'
[]
