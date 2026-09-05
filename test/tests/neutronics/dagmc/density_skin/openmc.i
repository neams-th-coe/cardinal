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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
  density_blocks = '1 2'
  power = 100.0

  skinner = moab
  cell_level = 0

  [Tallies]
    [Mesh]
      type = CellTally
      block = '1 2'
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp density'
    fields_min = '0 ${fparse rho0 - drho}'
    fields_max = '1000 ${fparse rho0 + 2 * drho}'
    n_field_bins = '1 4'
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
