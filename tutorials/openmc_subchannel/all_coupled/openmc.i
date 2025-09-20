!include ../common.i

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/mesh_in.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = ${power}
  lowest_cell_level = 2
  scaling = 100
  temperature_blocks = 'helium fuel clad sodium'
  density_blocks = 'sodium'
  xml_directory = '../openmc'

  [Tallies]
    [power]
      type = CellTally
      score = 'kappa_fission'
    []
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = ${inlet_temperature}
  []
  [density]
    type = ConstantIC
    variable = density
    value = ${fparse 1.00423e3 + -0.21390*inlet_temperature+-1.1046e-5*inlet_temperature^2}
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
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [conduction]
    type = TransientMultiApp
    input_files = 'conduction.i'
    sub_cycling = true
    execute_on = timestep_end
  []
  [subchannel]
    type = FullSolveMultiApp
    input_files = 'subchannel.i'
    max_procs_per_app = 1
    execute_on = timestep_begin
  []
[]

[Transfers]
  [power_to_conduction]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = conduction
    source_variable = kappa_fission
    variable = heat_source
    from_postprocessors_to_be_preserved = openmc_power_integral
    to_postprocessors_to_be_preserved = conduction_power_integral
  []
  [solid_temperature_from_conduction]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = conduction
    source_variable = T
    variable = temp
    to_blocks = 'fuel clad helium'
  []

  [linear_heat_rate_to_subchannel]
    type = MultiAppGeneralFieldNearestLocationTransfer
    variable = q_prime
    source_variable = q_prime
    from_multi_app = conduction
    to_multi_app = subchannel
    greedy_search = true
    use_bounding_boxes = false
    to_blocks = 'fuel_pins'
  []
  [fluid_temperature_from_subchannel]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = T
    variable = temp
    from_multi_app = subchannel
    greedy_search = true
    use_bounding_boxes = false
    to_blocks = 'sodium'
    from_blocks = 'subchannel'
  []
  [fluid_density_from_subchannel]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = rho
    variable = density
    from_multi_app = subchannel
    greedy_search = true
    use_bounding_boxes = false
    to_blocks = 'sodium'
    from_blocks = 'subchannel'
  []
  [clad_surface_temperature_to_conduction]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = subchannel
    to_multi_app = conduction
    source_variable = Tpin
    variable = T_wall
  []
[]

[Postprocessors]
  [openmc_power_integral]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    execute_on = 'transfer timestep_end'
  []
[]
