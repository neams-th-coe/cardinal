[GlobalParams]
  use_displaced_mesh = true
  displacements = 'd_x d_y d_z'
[]

[Mesh]
  [f]
    type = FileMeshGenerator
    file = volume.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  scaling = 100.0
  power = 100

  cell_level = 0
  temperature_blocks = 'left right'
  skinner = skin

  [Tallies]
    [cell]
      type = CellTally
      blocks = 'left right'
    []
  []
[]

[AuxVariables]
  [d_x]
  []
  [d_y]
  []
  [d_z]
  []

  [mapped_vol]
    family = MONOMIAL
    order = CONSTANT
  []
  [actual_openmc_vol]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 1000
  []
[]

[AuxKernels]
  [d_x]
    type = FunctionAux
    variable = d_x
    function = d_x
  []
  [mapped_vol]
    type = CellVolumeAux
    variable = mapped_vol
    volume_type = mapped
  []
  [actual_openmc_vol]
    type = CellVolumeAux
    variable = actual_openmc_vol
    volume_type = actual
  []
[]

[Functions]
  [d_x]
    type = ParsedFunction
    expression = 'exp((x+0.05))-1'
  []
[]

[UserObjects]
  [skin]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 1
    build_graveyard = true
    temperature_min = 0
    temperature_max = 2000
    output_skins = true
    verbose = True
  []
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 100000
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [vol_left]
    type = VolumePostprocessor
    block = 'left'
  []
  [vol_right]
    type = VolumePostprocessor
    block = 'right'
  []
[]

[Outputs]
  exodus = true
[]
