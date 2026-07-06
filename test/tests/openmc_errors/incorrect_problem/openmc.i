[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
  [max_tally_rel_err]
    type = TallyRelativeError
    value_type = max
  []
[]
