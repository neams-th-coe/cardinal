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

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100'
  cell_level = 0

  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      block = '100'

      check_tally_sum = false
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
  [reactivity_collision]
    type = Reactivity
    value_type = 'collision'
  []
  [reactivity_absorption]
    type = Reactivity
    value_type = 'absorption'
  []
  [reactivity_tracklength]
    type = Reactivity
    value_type = 'tracklength'
  []
  [reactivity_combined]
    type = Reactivity
    value_type = 'combined'
  []
[]

[Outputs]
  csv = true
[]
