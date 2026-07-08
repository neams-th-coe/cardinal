[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    nx = 1
    ny = 1
    nz = 1
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem

  power = 1
  cell_level = 0

  [Tallies/cell]
    type = CellTally
    normalize_by_global_tally = false
    score = 'heating heating_local fission_q_prompt fission_q_recoverable damage_energy
             H3_production absorption scatter nu_scatter prompt_nu_fission
             delayed_nu_fission decay_rate inverse_velocity'
  []
[]

[Executioner]
  type = Steady
[]
