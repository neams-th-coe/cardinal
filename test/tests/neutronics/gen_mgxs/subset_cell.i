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

  [MGXS]
    tally_type = cell
    particle = neutron
    group_structure = CASMO_2
    estimator = 'analog'
    normalize_by_global_tally = false
    hide_tally_vars = true

    add_scattering = false
    legendre_order = 0
    transport_correction = true

    add_fission = false

    add_fission_heating = true

    add_inverse_velocity = true

    add_diffusion_coefficient = false

    add_absorption = true
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [abs_xs_g1]
    type = PointValue
    point = '0 0 0'
    variable = abs_xs_g1
  []
  [abs_xs_g2]
    type = PointValue
    point = '0 0 0'
    variable = abs_xs_g2
  []
  [inv_v_g1]
    type = PointValue
    point = '0 0 0'
    variable = inv_v_g1
  []
  [inv_v_g2]
    type = PointValue
    point = '0 0 0'
    variable = inv_v_g2
  []
  [kappa_fission_g1]
    type = PointValue
    point = '0 0 0'
    variable = kappa_fission_g1
  []
  [kappa_fission_g2]
    type = PointValue
    point = '0 0 0'
    variable = kappa_fission_g2
  []
  [total_xs_g1]
    type = PointValue
    point = '0 0 0'
    variable = total_xs_g1
  []
  [total_xs_g2]
    type = PointValue
    point = '0 0 0'
    variable = total_xs_g2
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
