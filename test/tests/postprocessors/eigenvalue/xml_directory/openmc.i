[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../../neutronics/meshes/sphere.e
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
  check_tally_sum = false

  xml_directory = './xml_settings/'

  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      block = '100'
    []
  []
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
    type = KEigenvalue
    output = 'std_dev'
    value_type = 'collision'
  []
  [k_absorption_std_dev]
    type = KEigenvalue
    output = 'std_dev'
    value_type = 'absorption'
  []
  [k_tracklength_std_dev]
    type = KEigenvalue
    output = 'std_dev'
    value_type = 'tracklength'
  []
  [k_combined_std_dev]
    type = KEigenvalue
    output = 'std_dev'
    value_type = 'combined'
  []
  [k_collision_rel]
    type = KEigenvalue
    output = 'rel_err'
    value_type = 'collision'
  []
  [k_absorption_rel]
    type = KEigenvalue
    output = 'rel_err'
    value_type = 'absorption'
  []
  [k_tracklength_rel]
    type = KEigenvalue
    output = 'rel_err'
    value_type = 'tracklength'
  []
  [k_combined_rel]
    type = KEigenvalue
    output = 'rel_err'
    value_type = 'combined'
  []
[]

[Outputs]
  csv = true
[]
