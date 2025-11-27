[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  power = 1.0
  source_rate_normalization = 'kappa_fission'

  [Tallies/Cell]
    type = CellTally
    block = '100'
    normalize_by_global_tally = false
  []

  [MGXS]
    tally_type = cell
    particle = neutron
    group_structure = CASMO_2
    estimator = 'analog'
    normalize_by_global_tally = false
    hide_tally_vars = true

    add_scattering = true
    legendre_order = 1
    transport_correction = false

    add_fission = false

    add_fission_heating = false

    add_inverse_velocity = false

    add_diffusion_coefficient = false

    add_absorption = false
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [scatter_xs_g1_gp1_l0]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g1_gp1_l0
  []
  [scatter_xs_g1_gp2_l0]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g1_gp2_l0
  []
  [scatter_xs_g2_gp1_l0]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g2_gp1_l0
  []
  [scatter_xs_g2_gp2_l0]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g2_gp2_l0
  []
  [scatter_xs_g1_gp1_l1]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g1_gp1_l1
  []
  [scatter_xs_g1_gp2_l1]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g1_gp2_l1
  []
  [scatter_xs_g2_gp1_l1]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g2_gp1_l1
  []
  [scatter_xs_g2_gp2_l1]
    type = PointValue
    point = '0 0 0'
    variable = scatter_xs_g2_gp2_l1
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
