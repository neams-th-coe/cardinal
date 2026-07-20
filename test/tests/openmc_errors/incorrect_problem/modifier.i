[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  type = FEProblem
  solve = false

  [ModelModifiers]
    [n]
      type = OpenMCCellMaterialFill
      cell_id = 1
      material_ids = '1'
    []
  []
[]

[Executioner]
  type = Transient
[]
