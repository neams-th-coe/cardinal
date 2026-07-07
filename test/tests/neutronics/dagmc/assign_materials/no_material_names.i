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
    temperature_min = 0
    temperature_max = 1000
    n_temperature_bins = 1
    temperature = temp
    material_blocks = '1 2'
  []
[]

[Executioner]
  type = Steady
[]
