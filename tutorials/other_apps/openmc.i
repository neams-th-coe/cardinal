[Mesh]
  [slab]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 20
    xmax = 1.0
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  scaling = 100.0
  source_strength = 1e18
  cell_level = 0

  [Tallies]
    [tritium]
      type = CellTally
      score = H3_production
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [tritium_production]
    type = ElementIntegralVariablePostprocessor
    variable = H3_production
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
