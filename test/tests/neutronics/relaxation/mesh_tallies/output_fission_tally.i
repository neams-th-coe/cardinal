[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = ../../meshes/sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []
  [set_block_ids]
    type = SubdomainIDGenerator
    input = repeat
    subdomain_id = 0
  []

  allow_renumbering = false
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = axial
    execute_on = initial
  []
[]

[Functions]
  [axial]
    type = ParsedFunction
    value = '500 + z / 0.10 * 100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  solid_blocks = '0'
  tally_type = mesh
  tally_name = heat_source
  mesh_template = ../../meshes/sphere_in_m.e
  mesh_translations = '0.0 0.0 0.02
                       0.0 0.0 0.06
                       0.0 0.0 0.10'
  solid_cell_level = 1
  scaling = 100.0

  output = 'unrelaxed_tally'
  relaxation = constant

  check_tally_sum = false
[]

# This auxvariable and auxkernel is only here to avoid a re-gold (due to a
# variable name change).
[AuxVariables]
  [fission_tally]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [copy]
    type = SelfAux
    variable = fission_tally
    v = heat_source_raw
  []
[]


[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
  hide = 'heat_source_raw'
[]
