[Mesh]
  type = FileMesh
  file = ../mesh_tallies/slab.e
  allow_renumbering = false
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

[AuxVariables]
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
[]

rho0 = 600.0
drho = 50.0

[Functions]
  [density]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse rho0 - drho}, if (x <= ${x1}, ${rho0}, if (x <= ${x2}, ${fparse rho0 + drho}, ${fparse rho0 + 2 * drho})))'
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[AuxKernels]
  [density]
    type = FunctionAux
    variable = density
    function = density
    execute_on = timestep_begin
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
  power = 100.0

  skinner = moab
  cell_level = 0
[]

[UserObjects]
  [moab]
    type = MoabSkinner

    # just one temperature bin
    temperature_min = 0.0
    temperature_max = 1000.0
    n_temperature_bins = 1
    temperature = temp

    density_min = ${fparse rho0 - drho}
    density_max = ${fparse rho0 + 2 * drho}
    n_density_bins = 4
    density = density

    build_graveyard = true
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
