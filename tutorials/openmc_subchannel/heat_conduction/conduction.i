!include ../common.i

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/mesh_in.e
  []
  [delete_coolant]
    type = BlockDeletionGenerator
    input = file
    block = 'sodium'
  []
[]

[Variables]
  [T]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
  [heat_source_fuel]
    type = CoupledForce
    variable = T
    v = heat_source
    block = 'fuel'
  []
[]

[Materials]
   [k_helium]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
    block = 'helium'
  []
   [k_fuel]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '2.0'
    block = 'fuel'
  []
   [k_clad]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10.0'
    block = 'clad'
  []
[]

[AuxVariables]
  [T_wall]
    initial_condition = ${inlet_temperature}
  []
  [heat_source]
    initial_condition = 1e8
    block = 'fuel'
  []
  [q_prime]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.0
    block = 'fuel'
  []
[]

[BCs]
  [cladding_outer_bc]
    type = MatchedValueBC
    variable = T
    v = T_wall
    boundary = '7'
  []
[]

[AuxKernels]
  [q_prime]
    type = SpatialUserObjectAux
    variable = q_prime
    user_object = q_prime_uo
    block = 'fuel'
  []
[]

[UserObjects]
  [q_prime_uo]
    type = NearestPointLayeredIntegral
    variable = heat_source
    block = 'fuel'
    direction = z
    points_file = '../pin_centers.txt'
    num_layers = ${n_layers}
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
