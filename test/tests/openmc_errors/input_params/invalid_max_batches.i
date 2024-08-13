[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  # there are only 10 inactive batches, so we should error
  max_batches = 8
  k_trigger = std_dev
  k_trigger_threshold = 1e-2
[]

[Executioner]
  type = Transient
[]
