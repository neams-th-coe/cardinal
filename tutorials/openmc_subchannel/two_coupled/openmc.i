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
[]

[Postprocessors]
  [openmc_power_integral]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    execute_on = 'transfer timestep_end'
  []
[]
