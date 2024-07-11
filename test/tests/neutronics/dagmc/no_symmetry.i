[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/tet_cube.e
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = ../meshes/tet_cube.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  temperature_blocks = '1'
  power = 1000.0
  skinner = moab
  symmetry_mapper = sym
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    temperature_min = 0.0
    temperature_max = 900.0
    n_temperature_bins = 1
    build_graveyard = true
  []
  [sym]
    type = SymmetryPointGenerator
    normal = '1.0 0.0 0.0'
  []
[]

[Executioner]
  type = Steady
[]
