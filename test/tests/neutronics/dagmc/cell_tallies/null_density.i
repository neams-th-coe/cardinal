[Mesh]
  type = FileMesh
  file = ../mesh_tallies/slab.e
  allow_renumbering = false
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

[AuxVariables]
  [cell_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_id]
    type = CellIDAux
    variable = cell_id
  []
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

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

[ICs]
  [density]
    type = ConstantIC
    variable = density
    value = 11000.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = cell
  tally_blocks = '1 2'
  fluid_blocks = '1 2'
  fluid_cell_level = 0
  power = 100.0

  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = ${fparse T0 - dT}
    temperature_max = ${fparse T0 + 2 * dT}
    n_temperature_bins = 4
    temperature = temp
    build_graveyard = true

    # Effectively no density skinning at all, because theres just one bin.
    # Results should be the same as if there was no density skinning enabled.
    density_min = 0.0
    density_max = 15000.0
    n_density_bins = 1
    density = density
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
  hide = 'density'
[]
