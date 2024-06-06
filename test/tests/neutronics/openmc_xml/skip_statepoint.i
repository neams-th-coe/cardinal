[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0
  tally_type = cell
  normalize_by_global_tally = false
  initial_properties = xml

  inactive_batches = 3
  batches = 8
  skip_statepoint = true
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
