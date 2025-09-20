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
  type = Steady
[]

[Outputs]
  exodus = true
[]
