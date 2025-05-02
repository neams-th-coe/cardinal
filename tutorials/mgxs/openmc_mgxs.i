[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
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

  # We aren't computing MGXS on the helium cells, so we need to disable
  # the tally sum check.
  check_tally_sum = false

  [MGXS]
    tally_type = cell
    particle = neutron
    group_structure = CASMO_2
    estimator = 'analog'
    block = '0 1 3 4 5 6 7 8 9 10'

    add_scattering = true
    legendre_order = 0
    transport_correction = true

    add_fission = true

    add_fission_heating = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = 'TIMESTEP_END'
[]
