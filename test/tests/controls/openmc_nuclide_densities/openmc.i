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

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = 0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  cell_level = 0
  power = 100.0
[]

[UserObjects]
  [mat1]
    type = OpenMCNuclideDensities
    material_id = 2
    names = 'U235 U238'

    # dummy values which do not get set, because the control will override these
    densities = '0.0 0.0'
  []
[]

[Controls]
  [c]
    type = OpenMCNuclideDensitiesControl
    user_object = mat1
    names = 'U235 U238'
    densities = '0.03 0.09'
  []
[]

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
