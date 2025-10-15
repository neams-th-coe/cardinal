[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.0
    xmax = 0.39218e-2
    ymin = 0.0
    ymax = 300.0e-2
    nx = 5
    ny = 20
  []

  coord_type = RZ
  rz_coord_axis = Y
[]

[AuxVariables]
  [cell_volume_moose]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_volume_openmc]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_volume_moose]
    type = CellVolumeAux
    variable = cell_volume_moose
    volume_type = mapped
  []
  [cell_volume_openmc]
    type = CellVolumeAux
    variable = cell_volume_openmc
    volume_type = actual
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  power = 50
  scaling = 100.0

  [Tallies]
    [p]
      type = CellTally
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
