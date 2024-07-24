[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[AuxVariables]
  [cell_volume]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_volume]
    type = CellVolumeAux
    variable = cell_volume
    volume_type = mapped
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'kappa_fission'

  # Missing some hits in the model (only tallying a single pebble instead of 3)
  check_tally_sum = false

  [Tallies]
    [Cell_1]
      type = CellTally
      score = 'kappa_fission'
      blocks = '100'
    []
    [Cell_2]
      type = CellTally
      score = 'flux'
      blocks = '100'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
