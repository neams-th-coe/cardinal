[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 8
    ny = 8
    nz = 8
    xmin = -12.5
    xmax = 87.5
    ymin = -12.5
    ymax = 37.5
    zmin = -12.5
    zmax = 12.5
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = cell
  tally_blocks = '0'

  solid_blocks = '0'
  solid_cell_level = 0
  power = 100.0
[]

[UserObjects]
  [mat1]
    type = OpenMCNuclideDensities
    material_id = 1
    names = 'U235'
    densities = '0.04'
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
