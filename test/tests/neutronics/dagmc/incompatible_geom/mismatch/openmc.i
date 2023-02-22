[Mesh]
  [load]
    type = FileMeshGenerator
    file = ../../mesh_tallies/slab.e
  []
  [merge]
    type = RenameBlockGenerator
    input = load
    old_block = '2'
    new_block = '1'
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  tally_type = mesh
  mesh_template = ../../mesh_tallies/slab.e

  solid_blocks = '1'
  solid_cell_level = 0
  power = 100.0

  initial_properties = xml
  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = 0.0
    temperature_max = 1000.0
    n_temperature_bins = 4
    temperature = temp
    build_graveyard = true
  []
[]

[Executioner]
  type = Steady
[]
