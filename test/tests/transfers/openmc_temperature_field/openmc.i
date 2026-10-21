[Mesh]
  [temperature_mesh]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 100.0
  []
[]

[Functions]
  [temp_ic_function]
    type = ParsedFunction
    expression = "100 * x + 10 * y + z"
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp_ic_function
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_field_transfer = true
  scaling = 100.0
  temperature_blocks = "0"
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
    input_files = "nek.i"
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [temp_to_openmc]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = nek
    variable = temp
    source_variable = temperature
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
  [openmc_temp_field]
    type = OpenMCTemperatureField
    execute_on = "timestep_end"
    sort_by = "cell_id"
  []
[]

[Executioner]
  type = Transient
  dt = 2.5e-3
  num_steps = 2
[]

[Outputs]
  csv = true
  hide = "synchronize"
[]
