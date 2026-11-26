# Same order-0 identity as reconstruct_order0.i, but with the expansion limits derived
# from the [Mesh] extent via 'use_displaced_mesh' instead of being given explicitly. This covers
# the path where SpatialLegendreAux must read its limits from the filter on every
# evaluation: a cached copy goes stale once a filter re-derives its limits, which would
# make the reconstruction return zero outside the original extent and silently lose part
# of the score.

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
    [Plain]
      type = CellTally
      score = 'scatter'
      name = 'scatter_plain'
      block = '100 200'
      estimator = 'collision'
    []
    [Expansion]
      type = CellTally
      score = 'scatter'
      name = 'scatter_exp'
      block = '100 200'
      filters = 'SpatialLegendre'
      estimator = 'collision'
    []
  []

  [Filters]
    [SpatialLegendre]
      type = SpatialLegendreFilter
      order = 0
      axis = z
      use_displaced_mesh = true
    []
  []
[]

[AuxVariables]
  [reconstructed]
    family = MONOMIAL
    order = CONSTANT
  []
  [difference]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [reconstruct]
    type = SpatialLegendreAux
    variable = reconstructed
    score = scatter
    tally = Expansion
    normalization = volumetric
    execute_on = 'timestep_end'
  []
  # A relative difference is compared so that the gold values are exactly zero to within
  # floating point roundoff, instead of golding platform-dependent roundoff noise.
  [difference]
    type = ParsedAux
    variable = difference
    coupled_variables = 'reconstructed scatter_plain'
    expression = 'if (scatter_plain > 0, (reconstructed - scatter_plain) / scatter_plain, 0)'
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [max_difference]
    type = ElementExtremeValue
    variable = difference
    value_type = max
  []
  [min_difference]
    type = ElementExtremeValue
    variable = difference
    value_type = min
  []
  # Confirms the reconstruction is actually non-trivial (i.e. the comparison above isn't
  # passing because both fields are zero). The tally magnitude itself varies slightly from
  # run to run, so this is reduced to a boolean rather than golding the raw value.
  [max_reconstructed]
    type = ElementExtremeValue
    variable = reconstructed
    value_type = max
    outputs = none
  []
  [reconstruction_is_nonzero]
    type = ParsedPostprocessor
    pp_names = 'max_reconstructed'
    expression = 'max_reconstructed > 1e10'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
