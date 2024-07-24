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

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = axial
    execute_on = initial
  []
[]

[Functions]
  [axial]
    type = ParsedFunction
    expression = '500 + z / 0.10 * 100'
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

  relaxation = constant
  relaxation_factor = 0.5

  # Missing some hits in the model (only tallying a single pebble instead of 3)
  check_tally_sum = false

  [Tallies]
    [Cell_1]
      type = CellTally
      score = kappa_fission
      blocks = '100'
    []
    [Cell_2]
      type = CellTally
      score = flux
      blocks = '100'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
