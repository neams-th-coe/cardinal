[Mesh]
  type = FileMesh
  file = ../mesh_tallies/slab.e
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

T0 = 600.0
dT = 50.0

[Functions]
  [density]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse T0 - dT}, if (x <= ${x1}, ${T0}, if (x <= ${x2}, ${fparse T0 - dT}, ${fparse T0 + 2 * dT})))'
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = density
    function = density
    execute_on = timestep_begin
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[Tallies]
  [Mesh]
    type = CellTally
    tally_blocks = '1 2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  temperature_blocks = '1 2'
  density_blocks = '1 2'
  cell_level = 0
  power = 100.0

  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    density_min = ${fparse T0 - dT}
    density_max = ${fparse T0 + 2 * dT}
    n_density_bins = 4
    density = density

    temperature = temp
    n_temperature_bins = 1
    temperature_min = 0.0
    temperature_max = 1000.0

    build_graveyard = true
    output_skins = true
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  hide = 'temp density cell_instance cell_id'
[]
