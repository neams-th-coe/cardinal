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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = cell
  tally_blocks = '0'

  cell_level = 0
  power = 100.0
[]

[UserObjects]
  [mat1]
    type = OpenMCNuclideDensities
    material_id = 1
    names = 'U235'
    densities = '0.01'
  []
[]

[Controls]
  [c]
    type = OpenMCNuclideDensitiesControl
    name = mat1
    names = 'U235'
    densities = '0.005'
  []
[]

# If this works properly, then the value of this postprocessor should equal 0.90726
[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
