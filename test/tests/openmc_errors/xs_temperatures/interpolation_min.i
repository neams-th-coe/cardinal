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

    # We expect an error because we use the interpolation method but we are lower
    # than the lowest available temperature
    value = -100.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '0'
  tally_type = cell
  tally_blocks = '0'
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
