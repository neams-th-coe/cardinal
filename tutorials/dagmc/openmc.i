[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []

  allow_renumbering = false
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0
  temperature_blocks = '2 3'

  power = 1000.0
  volume_calculation = vol

  [Tallies]
    [heat_source]
      type = MeshTally
      mesh_template = mesh_in.e
      name = heat_source

      check_tally_sum = false
      normalize_by_global_tally = false
    []
  []
[]

[UserObjects]
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 200000
  []
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
[]
