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

  temperature_blocks = '1'
  cell_level = 0
  power = 100.0

  initial_properties = xml
  skinner = moab

  [Tallies]
    [Mesh]
      type = MeshTally
      mesh_template = ../../mesh_tallies/slab.e
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp'
    fields_min = '0'
    fields_max = '1000'
    n_field_bins = '4'
    build_graveyard = true
  []
[]

[Executioner]
  type = Steady
[]
