# Verifies 'max_order', which truncates the reconstruction below the order the filter
# tallied.
#
# The defining property is that Legendre moments do not depend on where the series is
# truncated: an order-L filter stores moments 0..L, so summing only 0..M (for M < L)
# must give EXACTLY the reconstruction an order-M filter would have produced. This is
# checked here without a second Monte Carlo run, by comparing two auxkernels reading the
# same order-3 tally:
#
#   * 'truncated'  : the order-3 filter evaluated with 'max_order = 0'
#   * 'full'       : the order-3 filter evaluated with every moment
#
# and separately confirming that 'truncated' matches an independent order-0 expansion
# tallied alongside it ('order0_ref'). Because both expansions are driven by the same
# particle histories in the same run, the l = 0 moments are identical and the comparison
# is exact to floating point roundoff, so the golded differences are exact zeros.

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
    # The order-3 expansion which 'max_order' truncates.
    [Expansion]
      type = CellTally
      score = 'scatter'
      name = 'scatter_exp'
      block = '100 200'
      filters = 'SpatialLegendre'
      estimator = 'collision'
    []
    # An independent order-0 expansion of the same score, used as the reference that the
    # truncated order-3 expansion must reproduce.
    [Order0]
      type = CellTally
      score = 'scatter'
      name = 'scatter_order0'
      block = '100 200'
      filters = 'Order0Legendre'
      estimator = 'collision'
    []
  []

  [Filters]
    [SpatialLegendre]
      type = SpatialLegendreFilter
      order = 3
      axis = z
      minimum = -2.0
      maximum = 10.0
    []
    [Order0Legendre]
      type = SpatialLegendreFilter
      order = 0
      axis = z
      minimum = -2.0
      maximum = 10.0
    []
  []
[]

[AuxVariables]
  [truncated]
    family = MONOMIAL
    order = CONSTANT
  []
  [full]
    family = MONOMIAL
    order = CONSTANT
  []
  [order0_ref]
    family = MONOMIAL
    order = CONSTANT
  []
  [truncation_difference]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  # The order-3 expansion truncated back to l = 0 only.
  [reconstruct_truncated]
    type = SpatialLegendreAux
    variable = truncated
    score = scatter
    tally = Expansion
    filter = SpatialLegendre
    normalization = volumetric
    max_order = 0
    execute_on = 'timestep_end'
  []
  # The same tally with all moments retained.
  [reconstruct_full]
    type = SpatialLegendreAux
    variable = full
    score = scatter
    tally = Expansion
    filter = SpatialLegendre
    normalization = volumetric
    execute_on = 'timestep_end'
  []
  # The independently tallied order-0 expansion.
  [reconstruct_order0]
    type = SpatialLegendreAux
    variable = order0_ref
    score = scatter
    tally = Order0
    filter = Order0Legendre
    normalization = volumetric
    execute_on = 'timestep_end'
  []
  # Relative difference, so the gold values are exact zeros rather than platform-dependent
  # roundoff noise.
  [truncation_difference]
    type = ParsedAux
    variable = truncation_difference
    coupled_variables = 'truncated order0_ref'
    expression = 'if (order0_ref > 0, (truncated - order0_ref) / order0_ref, 0)'
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  # Truncating the order-3 expansion at l = 0 must reproduce the independent order-0
  # expansion exactly.
  [max_truncation_difference]
    type = ElementExtremeValue
    variable = truncation_difference
    value_type = max
  []
  [min_truncation_difference]
    type = ElementExtremeValue
    variable = truncation_difference
    value_type = min
  []

  [int_truncated]
    type = ElementIntegralVariablePostprocessor
    variable = truncated
    execute_on = 'timestep_end'
    outputs = none
  []
  [int_full]
    type = ElementIntegralVariablePostprocessor
    variable = full
    execute_on = 'timestep_end'
    outputs = none
  []

  # Confirms the reconstruction is non-trivial, so the comparison above is not 0 == 0.
  [reconstruction_is_nonzero]
    type = ParsedPostprocessor
    pp_names = 'int_truncated'
    expression = 'abs(int_truncated) > 1e10'
    execute_on = 'timestep_end'
  []

  # 'max_order' must actually change the result: the truncated order-0 series must differ
  # from the full order-3 series. Without this, the test above would still pass if
  # 'max_order' were silently ignored AND the higher moments happened to be negligible.
  [truncation_changes_result]
    type = ParsedPostprocessor
    pp_names = 'int_truncated int_full'
    expression = 'abs(int_truncated - int_full) / abs(int_full) > 1e-6'
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
