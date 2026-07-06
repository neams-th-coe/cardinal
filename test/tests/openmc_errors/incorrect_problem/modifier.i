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
    type = OpenMCCellMaterialFill
    cell_id = 1
  []
[]

[Executioner]
  type = Transient
[]
