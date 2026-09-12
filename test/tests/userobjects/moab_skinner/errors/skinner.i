[Mesh]
  [cube]
    type = FileMeshGenerator
    file = ../../../neutronics/meshes/tet_cube.e
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [temp]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 500.0
  []
  [rho]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1000.0
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp rho'
    fields_min = '400 900'
    fields_max = '650 2000'
    n_field_bins = '5 3'
    material_blocks = "1"
    material_names = "mat"



    build_graveyard = true
    graveyard_scale_inner = 1.05
    graveyard_scale_outer = 1.25
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
