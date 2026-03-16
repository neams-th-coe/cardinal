!include common.i

[Mesh]
  [File]
    type = FileMeshGenerator
    file = solid_mesh_in.e
  []
[]

[Variables]
  [temp]
    initial_condition = ${T_AVG}
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
    block = 'uo2_tri uo2'
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
    block = 'uo2_tri uo2 clad'
  []
  [heat]
    type = CoupledForce
    variable = temp
    v = heat_source
    block = 'uo2_tri uo2'
  []
[]

[Functions]
  [T_fluid]
    type = ParsedFunction
    expression = '${T_INLET} + ${fparse T_OUTLET - T_INLET} * ((z + ${fparse 0.5 * 1e-2 * HEIGHT}) / ${fparse 1e-2 * HEIGHT})'
  []
[]

[BCs]
  [surface]
    type = ConvectiveFluxFunction
    T_infinity = T_fluid

    coefficient = ${CONV_HT_COEFF}
    variable = temp
    boundary = 'clad_or'
  []
[]

[Materials]
  [k_fuel]
    type = GenericConstantMaterial
    prop_values = '${K_FUEL}'
    prop_names = 'thermal_conductivity'
    block = 'uo2_tri uo2'
  []
  [k_clad]
    type = GenericConstantMaterial
    prop_values = '${K_CLAD}'
    prop_names = 'thermal_conductivity'
    block = 'clad'
  []
[]

[ThermalContact]
  # This adds boundary conditions between the fuel and the cladding, which represents
  # the heat flux in both directions as
  # q''= h * (T_1 - T_2)
  # where h is a conductance that accounts for conduction through a material and
  # radiation between two infinite parallel plate gray bodies.
  [one_to_two]
    type = GapHeatTransfer
    variable = temp
    primary = 'fuel_or'
    secondary = 'clad_ir'

    # we will use a quadrature-based approach to find the gap width and cross-side temperature
    quadrature = true

    # emissivity of the fuel
    emissivity_primary = 0.8

    # emissivity of the clad
    emissivity_secondary = 0.8

    # thermal conductivity of the gap material
    gap_conductivity = ${K_GAS}

    # geometric terms related to the gap
    gap_geometry_type = CYLINDER
    cylinder_axis_point_1 = '0 0 ${fparse -0.5 * 1e-2 * HEIGHT}'
    cylinder_axis_point_2 = '0 0 ${fparse  0.5 * 1e-2 * HEIGHT}'
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = 'TRANSFER TIMESTEP_END'
    block = 'uo2_tri uo2'
  []
  [max_fuel_T]
    type = NodalExtremeValue
    variable = temp
    block = 'uo2_tri uo2'
  []
  [max_clad_T]
    type = NodalExtremeValue
    variable = temp
    block = 'clad'
  []
[]

[Outputs]
  exodus = true
  execute_on = 'TIMESTEP_END'
  csv = true
[]
