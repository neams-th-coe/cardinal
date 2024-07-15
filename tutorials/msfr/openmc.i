[Mesh]
  [mesh]
    type = FileMeshGenerator
    file = msr.e
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 948.0
  []
  [density]
    type = ConstantIC
    variable = density
    value = ${fparse -0.882*948+4983.6}
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
  [density]
    type = ParsedAux
    variable = density
    expression = '-0.882*temp+4983.6'
    coupled_variables = temp
    execute_on = timestep_begin
  []
[]

[Tallies]
  [heat_source]
    type = MeshTally
    mesh_template = msr.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  output = unrelaxed_tally_std_dev
  verbose = true

  # this will start each Picard iteration from the fission source from the previous one
  reuse_source = true

  scaling = 100.0

  density_blocks = '1'
  temperature_blocks = '1'
  cell_level = 0

  power = 300.0e6

  relaxation = dufek_gudowski
  first_iteration_particles = 5000

  skinner = moab
[]

nb = 15.0
tmin = 800.0
tmax = 1150.0

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = ${nb}
    temperature_min = ${tmin}
    temperature_max = ${tmax}

    density = density
    n_density_bins = ${nb}
    density_min = ${fparse -0.882*tmax+4983.6}
    density_max = ${fparse -0.882*tmin+4983.6}

    build_graveyard = true
    output_skins = true
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
  [temp_to_openmc]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = nek
    variable = temp
    source_variable = temp
  []
  [power_to_nek]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = nek
    source_variable = kappa_fission
    variable = heat_source
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
  [k]
    type = KEigenvalue
  []
  [k_std_dev]
    type = KStandardDeviation
  []
  [max_tally_err]
    type = TallyRelativeError
  []
  [max_T]
    type = ElementExtremeValue
    variable = temp
  []
  [avg_T]
    type = ElementAverageValue
    variable = temp
  []
  [max_q]
    type = ElementExtremeValue
    variable = kappa_fission
  []
  [synchronize]
    type = Receiver
    default = 1.0
  []
[]

t_nek = ${fparse 2e-4 * 7.669}

[Executioner]
  type = Transient
  dt = ${fparse 2000 * t_nek}
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'synchronize'
[]
