[Mesh]
  [openmc_mesh]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 10.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 1000.0
  temperature_blocks = '0'
  cell_level = 0
  scaling = 10.0
  
  [Tallies]
    [power]
      type = CellTally
      name = kappa_fission
    []
  []
[]

[Postprocessors]
  [integrated_power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
