# A SpatialLegendreAux which omits 'filter' on a tally carrying two
# SpatialLegendreFilter(s). The object cannot know which expansion to evaluate, so
# construction must error and ask for 'filter'. This is a dedicated input rather than a
# cli_args override of max_order.i because the condition requires 'filter' to be ABSENT,
# and cli_args can only set a parameter (setting it to an empty string instead searches
# for a filter literally named '').

[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
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
  [fluid]
    type = FileMeshGenerator
    file = ../../heat_source/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Heating]
      type = CellTally
      score = 'kappa_fission'
      block = '100 200'
    []
    [Expansion]
      type = CellTally
      score = 'scatter'
      name = 'scatter_exp'
      block = '100 200'
      filters = 'FirstLegendre SecondLegendre'
      estimator = 'collision'
    []
  []

  [Filters]
    [FirstLegendre]
      type = SpatialLegendreFilter
      order = 1
      axis = z
      minimum = -2.0
      maximum = 10.0
    []
    [SecondLegendre]
      type = SpatialLegendreFilter
      order = 2
      axis = x
      minimum = -2.0
      maximum = 10.0
    []
  []
[]

[AuxVariables]
  [reconstructed]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  # Deliberately omits 'filter'.
  [reconstruct]
    type = SpatialLegendreAux
    variable = reconstructed
    score = scatter
    tally = Expansion
    normalization = volumetric
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
