# Verifies the element-local Legendre expansion. A SpatialLegendreFilter restricted to
# a single cell (block 200) with 'use_displaced_mesh' builds a basis spanning only that cell, so
# the order-2 expansion is a genuine sub-cell shape. Reconstructing it with
# 'normalization = volumetric' must conserve the integral of the score: the tally
# variables Cardinal writes are already divided by the mapped volume (which carries the
# length of the expansion domain), so the correct series weight is (2l + 1) and NOT
# (2l + 1) / (maximum - minimum). With the wrong weight the integral ratio is ~0.08.

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

  # Zero displacements (see the AuxVariables below), declared so that a displaced problem
  # exists: MOOSE resets 'use_displaced_mesh' to false for any object created through
  # addObject when the [Mesh] declares no 'displacements'.
  displacements = 'disp_x disp_y disp_z'
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
      block = '200'
      check_tally_sum = false
    []
    [Plain]
      type = CellTally
      score = 'scatter'
      name = 'scatter_plain'
      block = '200'
      check_tally_sum = false
      estimator = 'collision'
    []
    [Expansion]
      type = CellTally
      score = 'scatter'
      name = 'scatter_exp'
      block = '200'
      check_tally_sum = false
      filters = 'SpatialLegendre'
      estimator = 'collision'
    []
  []

  [Filters]
    [SpatialLegendre]
      type = SpatialLegendreFilter
      order = 2
      axis = z
      use_displaced_mesh = true
      block = '200'
    []
  []
[]

[AuxVariables]
  [reconstructed]
    family = MONOMIAL
    order = CONSTANT
    block = '200'
  []
  # Zero displacements, present only so that a displaced problem is created. The expansion
  # limits therefore match the undisplaced extent of block 200.
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxKernels]
  [reconstruct]
    type = SpatialLegendreAux
    variable = reconstructed
    block = '200'
    score = scatter
    tally = Expansion
    normalization = volumetric
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [int_rec]
    type = ElementIntegralVariablePostprocessor
    variable = reconstructed
    block = '200'
    outputs = none
  []
  [int_plain]
    type = ElementIntegralVariablePostprocessor
    variable = scatter_plain
    block = '200'
    outputs = none
  []
  # Must be 1 to within the Monte Carlo noise of the two tallies.
  [integral_ratio]
    type = ParsedPostprocessor
    pp_names = 'int_rec int_plain'
    expression = 'int_rec / int_plain'
    outputs = none
  []
  # The reconstruction must conserve the integral of the score. 1% tolerance covers the
  # Monte Carlo noise of the two independent tallies; the wrong series weight gives 0.08.
  [integral_conserved]
    type = ParsedPostprocessor
    pp_names = 'integral_ratio'
    expression = 'abs(integral_ratio - 1) < 0.01'
  []
  # Confirms the reconstruction is actually non-trivial (i.e. the comparison above isn't
  # passing because both fields are zero). The tally magnitude itself varies slightly from
  # run to run, so this is reduced to a boolean rather than golding the raw value.
  [max_reconstructed]
    block = '200'
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
