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
    material_names = "mat"

    temperature = temp
    n_temperature_bins = 5
    temperature_min = 400.0
    temperature_max = 650.0

    density = rho
    n_density_bins = 3
    density_min = 900.0
    density_max = 2000.0

    build_graveyard = true
    graveyard_scale_inner = 1.05
    graveyard_scale_outer = 1.25
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
