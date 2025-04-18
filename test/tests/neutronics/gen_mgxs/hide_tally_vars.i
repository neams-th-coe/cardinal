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
  # Some tally results will be missed as photon transport is enabled, but we filter MGXS tallies by a single particle type.
  # This forces us to disable global normalization.
  normalize_by_global_tally = false

  [MGXS]
    tally_type = cell
    particle = neutron
    energy_boundaries = '1e7 0.0'
    estimator = 'tracklength'

    add_scattering = true
    legendre_order = 0
    transport_correction = false

    add_fission = true

    add_fission_heating = true

    add_inverse_velocity = true

    add_diffusion_coefficient = true

    add_absorption = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
[]
