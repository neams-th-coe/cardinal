[Mesh]
  type = FileMesh
  file = ../mesh_tallies/slab.e
  allow_renumbering = false
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

T0 = 600.0
dT = 50.0

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse T0 - dT}, if (x <= ${x1}, ${T0}, if (x <= ${x2}, ${fparse T0 + dT}, ${fparse T0 + 2 * dT})))'
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = timestep_begin
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
  cell_level = 0
  power = 100.0

  skinner = moab

  [Tallies]
    [Cell]
      type = CellTally
      block = '1 2'
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = ${fparse T0 - dT}
    temperature_max = ${fparse T0 + 2 * dT}
    n_temperature_bins = 4
    temperature = temp
    build_graveyard = true
    implicit_complement_material="h2o"
    graveyard_scale_inner = 1.1
    graveyard_scale_outer = 1.2
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [k_std_dev]
    type = KEigenvalue
    output = 'std_dev'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
