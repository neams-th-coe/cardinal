l = 0.2
h = 1.0

[Mesh]
  [solid]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = ${fparse -l / 2.0}
    xmax = ${fparse l / 2.0}
    ymin = ${fparse -l / 2.0}
    ymax = ${fparse l / 2.0}
    zmin = 0.0
    zmax = ${h}
    nx = 4
    ny = 4
    nz = 20
  []
  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = solid
    combinatorial_geometry = 'z < ${fparse h / 2.0}'
    block_id = 100
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = 450.0
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '0 100'
    tally_score = heating_local
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  scaling = 100.0
  temperature_blocks = '0 100'
  lowest_cell_level = 1
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  exodus = true
[]
