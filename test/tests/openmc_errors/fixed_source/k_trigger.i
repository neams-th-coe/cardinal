[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  k_trigger = std_dev
  k_trigger_threshold = 1e-4
  max_batches = 100
  source_strength = 1e6
[]

[Executioner]
  type = Transient
[]
