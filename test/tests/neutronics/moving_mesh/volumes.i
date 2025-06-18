[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y disp_z'
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
  temperature_blocks = 'left right'
  cell_level = 0
  skinner = skin
  volume_calculation = vol
  verbose = true
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
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
  [disp_x] # this executes after the [Problem]
    type = FunctionAux
    variable = disp_x
    function = disp_x
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
  [disp_x]
    type = ParsedFunction
    expression = 'exp((x+0.05))-1'
  []
[]

[UserObjects]
  [skin]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 1
    temperature_min = 0
    temperature_max = 2000
    build_graveyard = true
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
