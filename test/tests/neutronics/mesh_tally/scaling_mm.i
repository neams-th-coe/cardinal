[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [smg]
    type = TransformGenerator
    input = solid_ids
    transform = 'SCALE'
    vector_value = '0.01 0.01 0.01'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = false

  power = 100.0
  scaling = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_rel_err]
    type = TallyRelativeError
    value_type = max
  []
  [min_rel_err]
    type = TallyRelativeError
    value_type = min
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
