[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0
  source_strength = 1e6
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]
