[Mesh]
  [m]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  tally_type = none
  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 2
    temperature_min = 0.0
    temperature_max = 1000.0
  []
[]

[Executioner]
  type = Steady
[]
