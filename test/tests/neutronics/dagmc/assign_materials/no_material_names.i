[Mesh]
  [slab]
    type = FileMeshGenerator
    file = ../mesh_tallies/slab.e
  []
  allow_renumbering = false
[]

[AuxVariables]
  [temp]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp'
    fields_min = '0'
    fields_max = '1000'
    n_field_bins = '1'
  []
[]

[Executioner]
  type = Steady
[]
