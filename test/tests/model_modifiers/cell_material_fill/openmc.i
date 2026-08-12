[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 8
    ny = 8
    nz = 5
    xmin = 0
    xmax = 20.0
    ymin = 0.0
    ymax = 20.0
    zmin = 0.0
    zmax = 100.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  [ModelModifiers]
    [one]
      type = OpenMCCellMaterialFill
      cell_id = 2
      material_ids = '1 1 1'
    []
  []
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
  csv = true
[]
