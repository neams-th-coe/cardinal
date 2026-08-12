[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 2
  ymin = -1
  ymax = 2
  zmin = -1
  zmax = 1
  nx = 20
  ny = 20
  nz = 20
[]

[Problem]
  type = OpenMCCellAverageProblem

  [ModelModifiers]
    [c]
      type = OpenMCCellMaterialFill
      cell_id = 2
      material_ids = '1'
    []
  []
[]

[Executioner]
  type = Steady
[]
