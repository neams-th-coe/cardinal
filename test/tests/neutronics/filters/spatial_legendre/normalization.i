# Verifies the three 'normalization' modes of SpatialLegendreAux against each other,
# using relationships that hold exactly and so do not depend on the cross section
# library or the Monte Carlo statistics.
#
# 'raw' weights moment l by (2l + 1) / (maximum - minimum), while 'volumetric' weights
# it by (2l + 1). Every term therefore differs by the same constant factor
# 1 / (maximum - minimum), so for ANY expansion order
#
#   integral(raw) / integral(volumetric) = 1 / (maximum - minimum) = 1 / 12
#
# exactly. 'none' applies no factor at all, so it agrees with 'volumetric' only for the
# l = 0 term (where 2l + 1 = 1); that case is checked separately by the 'none_order0'
# test, which drops the filter to order 0 via cli_args.

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
      filters = 'SpatialLegendre'
      estimator = 'collision'
    []
  []

  [Filters]
    [SpatialLegendre]
      type = SpatialLegendreFilter
      order = 2
      axis = z
      minimum = -2.0
      maximum = 10.0
    []
  []
[]

[AuxVariables]
  [volumetric]
    family = MONOMIAL
    order = CONSTANT
  []
  [raw]
    family = MONOMIAL
    order = CONSTANT
  []
  [none]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [reconstruct_volumetric]
    type = SpatialLegendreAux
    variable = volumetric
    score = scatter
    tally = Expansion
    normalization = volumetric
    execute_on = 'timestep_end'
  []
  [reconstruct_raw]
    type = SpatialLegendreAux
    variable = raw
    score = scatter
    tally = Expansion
    normalization = raw
    execute_on = 'timestep_end'
  []
  [reconstruct_none]
    type = SpatialLegendreAux
    variable = none
    score = scatter
    tally = Expansion
    normalization = none
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [int_volumetric]
    type = ElementIntegralVariablePostprocessor
    variable = volumetric
    execute_on = 'timestep_end'
    outputs = none
  []
  [int_raw]
    type = ElementIntegralVariablePostprocessor
    variable = raw
    execute_on = 'timestep_end'
    outputs = none
  []
  [int_none]
    type = ElementIntegralVariablePostprocessor
    variable = none
    execute_on = 'timestep_end'
    outputs = none
  []

  # 'raw' differs from 'volumetric' by exactly 1 / (maximum - minimum) = 1 / 12 for every
  # moment, so this ratio is an exact constant independent of the tally values.
  [raw_over_volumetric]
    type = ParsedPostprocessor
    pp_names = 'int_raw int_volumetric'
    expression = 'int_raw / int_volumetric'
    execute_on = 'timestep_end'
  []

  # Confirms the reconstruction is non-trivial, so the ratio above is not 0/0. The tally
  # magnitude varies slightly from run to run, so this is reduced to a boolean.
  [reconstruction_is_nonzero]
    type = ParsedPostprocessor
    pp_names = 'int_volumetric'
    expression = 'abs(int_volumetric) > 1e10'
    execute_on = 'timestep_end'
  []

  # For an order-2 expansion 'none' must NOT coincide with 'volumetric', since the
  # l = 1 and l = 2 moments are weighted by 3 and 5 rather than 1. This guards against
  # the normalization switch silently falling through to a single branch.
  [none_differs_from_volumetric]
    type = ParsedPostprocessor
    pp_names = 'int_none int_volumetric'
    expression = 'abs(int_none - int_volumetric) / abs(int_volumetric) > 1e-6'
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
