[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -2
  xmax = 2
  ymin = -2
  ymax = 2
  zmin = -2
  zmax = 10
  nx = 5
  ny = 5
  nz = 5
[]

[AuxVariables]
  [dummy]
  []
[]

[AuxKernels]
  [dummy]
    type = ConstantAux
    variable = dummy
    value = 0.0
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '0'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '0'
  density_blocks = '0'
  verbose = true
  cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
