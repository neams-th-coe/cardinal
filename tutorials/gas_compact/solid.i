compact_diameter = 0.0127                # diameter of fuel compacts (m)
height = 1.60                            # height of the unit cell (m)
triso_pf = 0.15                          # TRISO packing fraction (%)
kernel_radius = 214.85e-6                # fissile kernel outer radius (m)
buffer_radius = 314.85e-6                # buffer outer radius (m)
iPyC_radius = 354.85e-6                  # inner PyC outer radius (m)
SiC_radius = 389.85e-6                   # SiC outer radius (m)
oPyC_radius = 429.85e-6                  # outer PyC outer radius (m)

# material parameters
fluid_Cp = 5189.0                        # fluid isobaric specific heat (J/kg/K)
buffer_k = 0.5                           # buffer thermal conductivity (W/m/K)
PyC_k = 4.0                              # PyC thermal conductivity (W/m/K)
SiC_k = 13.9                             # SiC thermal conductivity (W/m/K)
kernel_k = 3.5                           # fissil kernel thermal conductivity (W/m/K)
matrix_k = 15.0                          # graphite matrix thermal conductivity (W/m/K)

# operating conditions
inlet_T = 598.0                          # inlet fluid temperature (K)
power = 30e3                             # unit cell power (W)
mdot = 0.011                             # fluid mass flowrate (kg/s)

# volume of the fuel compacts (six 1/3-compacts)
compact_vol = ${fparse 2 * pi * compact_diameter * compact_diameter / 4.0 * height}

# compute the volume fraction of each TRISO layer in a TRISO particle
# for use in computing average thermophysical properties
kernel_fraction = ${fparse kernel_radius^3 / oPyC_radius^3}
buffer_fraction = ${fparse (buffer_radius^3 - kernel_radius^3) / oPyC_radius^3}
ipyc_fraction = ${fparse (iPyC_radius^3 - buffer_radius^3) / oPyC_radius^3}
sic_fraction = ${fparse (SiC_radius^3 - iPyC_radius^3) / oPyC_radius^3}
opyc_fraction = ${fparse (oPyC_radius^3 - SiC_radius^3) / oPyC_radius^3}

# multiplicative factor on assumed heat source distribution to get correct magnitude
q0 = ${fparse power / (4.0 * height * compact_diameter * compact_diameter / 4.0)}

[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[Variables]
  [T]
    initial_condition = ${inlet_T}
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = T
  []
  [source]
    type = CoupledForce
    variable = T
    v = power
    block = 'compacts compacts_trimmer_tri'
  []
[]

[BCs]
  [pin_outer]
    type = MatchedValueBC
    variable = T
    v = fluid_temp
    boundary = 'fluid_solid_interface'
  []
[]

[AuxVariables]
  [fluid_temp]
  []
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [fluid_temp]
    type = FunctionIC
    variable = fluid_temp
    function = axial_fluid_temp
  []
[]

[Functions]
  [k_graphite]
    type = ParsedFunction
    expression = '${matrix_k}'
  []
  [k_TRISO]
    type = ParsedFunction
    expression = '${kernel_fraction} * ${kernel_k} + ${buffer_fraction} * ${buffer_k} + ${fparse ipyc_fraction + opyc_fraction} * ${PyC_k} + ${sic_fraction} * ${SiC_k}'
  []
  [k_compacts]
    type = ParsedFunction
    expression = '${triso_pf} * k_TRISO + ${fparse 1.0 - triso_pf} * k_graphite'
    symbol_names = 'k_TRISO k_graphite'
    symbol_values = 'k_TRISO k_graphite'
  []
  [axial_fluid_temp]
    type = ParsedFunction
    expression = '${q0} * ${compact_vol} * (${height} - ${height} * cos(pi * z / ${height})) / pi / ${mdot} / ${fluid_Cp} + ${inlet_T}'
  []
[]

[Materials]
  [graphite]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = k_graphite
    temp = T
    block = 'graphite'
  []
  [compacts]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = k_TRISO
    temp = T
    block = 'compacts compacts_trimmer_tri'
  []
[]

[Postprocessors]
  [max_fuel_T]
    type = ElementExtremeValue
    variable = T
    value_type = max
    block = 'compacts compacts_trimmer_tri'
  []
  [max_block_T]
    type = ElementExtremeValue
    variable = T
    value_type = max
    block = 'graphite'
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = 'compacts compacts_trimmer_tri'
    execute_on = transfer
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

[UserObjects]
  [average_fuel_axial]
    type = NearestPointLayeredAverage
    variable = T
    points = '0.0 0.0 0.0'
    direction = z
    num_layers = 30
    block = 'compacts compacts_trimmer_tri'
  []
  [average_block_axial]
    type = NearestPointLayeredAverage
    variable = T
    points = '0.0 0.0 0.0'
    direction = z
    num_layers = 30
    block = 'graphite compacts_trimmer_tri'
  []
[]

[VectorPostprocessors]
  [fuel_axial_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_fuel_axial
  []
  [block_axial_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_block_axial
  []
[]
