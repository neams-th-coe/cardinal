[GlobalParams]
  use_displaced_mesh = true
  displacements = 'd_x d_y d_z'
[]

[Mesh]
  [f]
    type = FileMeshGenerator
    file = ../volume.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  scaling = 100.0
  cell_level = 0
  temperature_blocks = 'left right'
  skinner = skin
  volume_calculation = vol
  verbose = true
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
    verbose = True
  []
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 1000000
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [moose_vol_left]
    type = VolumePostprocessor
    block = 'left'
  []
  [moose_vol_right]
    type = VolumePostprocessor
    block = 'right'
  []
  [mapped_vol_left]
    type = ElementExtremeValue
    variable = mapped_vol
    block = 'left'
  []
  [mapped_vol_right]
    type = ElementExtremeValue
    variable = mapped_vol
    block = 'right'
  []
  [openmc_vol_left]
    type = ElementExtremeValue
    variable = actual_openmc_vol
    block = 'left'
  []
  [openmc_vol_right]
    type = ElementExtremeValue
    variable = actual_openmc_vol
    block = 'right'
  []
[]

[Outputs]
  csv = true
[]
