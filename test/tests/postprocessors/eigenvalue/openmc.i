[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
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

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100'
  cell_level = 0
  check_tally_sum = false

  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [k_collision]
    type = KEigenvalue
    value_type = 'collision'
  []
  [k_absorption]
    type = KEigenvalue
    value_type = 'absorption'
  []
  [k_tracklength]
    type = KEigenvalue
    value_type = 'tracklength'
  []
  [k_combined]
    type = KEigenvalue
    value_type = 'combined'
  []
  [k_collision_std_dev]
    type = KStandardDeviation
    value_type = 'collision'
  []
  [k_absorption_std_dev]
    type = KStandardDeviation
    value_type = 'absorption'
  []
  [k_tracklength_std_dev]
    type = KStandardDeviation
    value_type = 'tracklength'
  []
  [k_combined_std_dev]
    type = KStandardDeviation
    value_type = 'combined'
  []
[]

[Outputs]
  csv = true
[]
