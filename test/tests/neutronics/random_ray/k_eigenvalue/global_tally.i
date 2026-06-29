[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -25.0
    xmax = 25.0
    ymin = -25.0
    ymax = 25.0
    zmin = 0.0
    zmax = 50.0
    nx = 1
    ny = 1
    nz = 10
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  power = 1.0
  source_rate_normalization = 'kappa_fission'

  batches = 100
  inactive_batches = 10
  particles = 1000

  [Tallies]
    [flux]
      type = CellTally
      score = 'flux'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      filters = 'energy'
      block = '0'
    []
    [other]
      type = CellTally
      score = 'kappa_fission total fission nu_fission'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      block = '0'
    []
  []
  [Filters/energy]
    type = EnergyFilter
    energy_boundaries = '1e-8 1e8'
  []
[]

[Executioner]
  type = Steady
[]
