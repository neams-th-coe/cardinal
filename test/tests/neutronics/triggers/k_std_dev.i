[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  k_trigger = std_dev
  k_trigger_threshold = 1.2e-2
  max_batches = 100
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [k_std_dev]
    type = KStandardDeviation
  []
[]

[Outputs]
  csv = true
[]
