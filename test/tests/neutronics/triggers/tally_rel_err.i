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
  power = 100.0
  temperature_blocks = '100'
  cell_level = 0
  tally_type = cell
  normalize_by_global_tally = false
  initial_properties = xml

  tally_trigger = rel_err
  tally_trigger_threshold = 2e-2
  max_batches = 200
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_tally_err]
    type = TallyRelativeError
  []
[]

[Outputs]
  csv = true
[]
