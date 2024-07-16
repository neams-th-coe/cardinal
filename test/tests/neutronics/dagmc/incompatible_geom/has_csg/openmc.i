[Mesh]
  type = FileMesh
  file = ../../mesh_tallies/slab.e
  allow_renumbering = false
  parallel_type = replicated
[]

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = ../../mesh_tallies/slab.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  initial_properties = xml
  power = 100.0
  cell_level = 0
  temperature_blocks = '1 2'
  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 2
    temperature_min = 0.0
    temperature_max = 1000.0
  []
[]

[Executioner]
  type = Steady
[]
