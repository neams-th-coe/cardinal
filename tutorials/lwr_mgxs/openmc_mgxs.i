[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[AuxVariables]
  [scatter_ratio_g1]
    type = MooseVariable
    family = MONOMIAL
    order = CONSTANT
    block = '0 1 3 4 5 6 7 8 9 10'
  []
  [scatter_ratio_g2]
    type = MooseVariable
    family = MONOMIAL
    order = CONSTANT
    block = '0 1 3 4 5 6 7 8 9 10'
  []
[]

[AuxKernels]
  [comp_scatter_ratio_g1]
    type = ParsedAux
    variable = scatter_ratio_g1
    coupled_variables = 'total_xs_g1 scatter_xs_g1_gp1_l0 scatter_xs_g1_gp2_l0'
    expression = '(scatter_xs_g1_gp1_l0 + scatter_xs_g1_gp2_l0) / total_xs_g1'
    block = '0 1 3 4 5 6 7 8 9 10'
  []
  [comp_scatter_ratio_g2]
    type = ParsedAux
    variable = scatter_ratio_g2
    coupled_variables = 'total_xs_g2 scatter_xs_g2_gp1_l0 scatter_xs_g2_gp2_l0'
    expression = '(scatter_xs_g2_gp1_l0 + scatter_xs_g2_gp2_l0) / total_xs_g2'
    block = '0 1 3 4 5 6 7 8 9 10'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  # Since we're generating multi-group cross sections the value
  # of power chosen is arbitrary.
  power = 1.0
  # We set the cell level to 1 to ensure we get multi-group cross
  # sections for each unique cell in the lattice.
  cell_level = 1

  verbose = true
  particles = 10000
  inactive_batches = 50
  batches = 1000

  # The MGXS block is adding fission heating, so we set the source
  # rate normalization to 'kappa_fission'.
  source_rate_normalization = 'kappa_fission'
[]

[Problem/MGXS]
  tally_type = cell
  group_structure = CASMO_2
  particle = neutron
  estimator = analog
  block = '0 1 3 4 5 6 7 8 9 10'

  add_fission = true

  add_fission_heating = true

  transport_correction = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = 'TIMESTEP_END'
[]

