[Mesh]
  type = FileMesh
  file = slab.e
  allow_renumbering = false
  parallel_type = replicated
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

T0 = 600.0
dT = 50.0

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse T0 - dT}, if (x <= ${x1}, ${T0}, if (x <= ${x2}, ${fparse T0 - dT}, ${fparse T0 + 2 * dT})))'
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

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = slab.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  temperature_blocks = '1 2'
  cell_level = 0
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
[]
