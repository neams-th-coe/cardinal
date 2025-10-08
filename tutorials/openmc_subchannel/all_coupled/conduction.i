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
    # OpenMC computes the variable as constant monomial, so we can receive the
    # field into exactly the same type here (not required, but this will be clearer to
    # visualize in paraview)
    family = MONOMIAL
    order = CONSTANT
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

[Executioner]
  type = Transient
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
    execute_on = 'initial timestep_begin'
  []
[]

[Postprocessors]
  [conduction_power_integral]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = 'fuel'
    execute_on = 'transfer'
  []
[]

[Outputs]
  exodus = true
[]
