!include common_input.i

# This input models conjugate heat transfer between MOOSE and NekRS for
# a single coolant channel. This input file should be run with:
#
# cardinal-opt -i solid_nek.i

unit_cell_power = ${fparse power / (n_bundles * n_coolant_channels_per_block) * unit_cell_height / height}
unit_cell_mdot = ${fparse mdot / (n_bundles * n_coolant_channels_per_block)}

# compute the volume fraction of each TRISO layer in a TRISO particle
# for use in computing average thermophysical properties
kernel_fraction = ${fparse kernel_radius^3 / oPyC_radius^3}
buffer_fraction = ${fparse (buffer_radius^3 - kernel_radius^3) / oPyC_radius^3}
ipyc_fraction = ${fparse (iPyC_radius^3 - buffer_radius^3) / oPyC_radius^3}
sic_fraction = ${fparse (SiC_radius^3 - iPyC_radius^3) / oPyC_radius^3}
opyc_fraction = ${fparse (oPyC_radius^3 - SiC_radius^3) / oPyC_radius^3}

# multiplicative factor on assumed heat source distribution to get correct magnitude
q0 = ${fparse unit_cell_power / (4.0 * unit_cell_height * compact_diameter * compact_diameter / 4.0)}

# Time step interval on which to exchange data between NekRS and MOOSE
N = 50
U_ref = ${fparse mdot / (n_bundles * n_coolant_channels_per_block) / fluid_density / (pi * channel_diameter * channel_diameter / 4.0)}
t0 = ${fparse channel_diameter / U_ref}
nek_dt = 6e-3


[Mesh]
  type = FileMesh
  file = solid_rotated.e
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
    block = 'compacts'
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

[ICs]
  [power]
    type = FunctionIC
    variable = power
    function = axial_power
    block = 'compacts'
  []
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
  [axial_power] # volumetric power density
    type = ParsedFunction
    expression = 'sin(pi * z / ${unit_cell_height}) * ${q0}'
  []
  [axial_fluid_temp]
    type = ParsedFunction
    expression = '${inlet_T} + z / ${unit_cell_height} * ${unit_cell_power} / ${unit_cell_mdot} / ${fluid_Cp}'
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
    thermal_conductivity_temperature_function = k_compacts
    temp = T
    block = 'compacts'
  []
[]

[Postprocessors]
  [flux_integral] # evaluate the total heat flux for normalization
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = 'fluid_solid_interface'
  []
  [max_fuel_T]
    type = ElementExtremeValue
    variable = T
    value_type = max
    block = 'compacts'
  []
  [max_block_T]
    type = ElementExtremeValue
    variable = T
    value_type = max
    block = 'graphite'
  []
  [max_flux]
    type = ElementExtremeValue
    variable = flux
  []
  [synchronization_to_nek]
    type = Receiver
    default = 1.0
  []

  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = 'compacts'
  []
[]

[AuxVariables]
  [fluid_temp]
  []
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    diffusion_variable = T
    component = normal
    diffusivity = thermal_conductivity
    variable = flux
    boundary = 'fluid_solid_interface'
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [heat_flux_to_nek]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    variable = flux
    from_boundaries = 'fluid_solid_interface'
    to_boundaries = '3'
    to_multi_app = nek
  []
  [flux_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
  [temperature_to_bison]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    variable = fluid_temp
    from_boundaries = '3'
    to_boundaries = 'fluid_solid_interface'
    from_multi_app = nek
  []
  [synchronization_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronization_to_nek
    to_multi_app = nek
  []
[]

[Executioner]
  type = Transient
  dt = ${fparse N * nek_dt * t0}

  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'

  steady_state_detection = true
  steady_state_tolerance = 1e-1
[]

[UserObjects]
  [average_fuel_axial]
    type = LayeredAverage
    variable = T
    direction = z
    num_layers = ${num_layers_for_plots}
    block = 'compacts'
  []
  [average_block_axial]
    type = LayeredAverage
    variable = T
    direction = z
    num_layers = ${num_layers_for_plots}
    block = 'graphite'
  []
  [average_flux_axial]
    type = LayeredSideAverage
    variable = flux
    direction = z
    num_layers = ${num_layers_for_plots}
    boundary = 'fluid_solid_interface'
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
  [flux_axial_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_flux_axial
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  hide = 'synchronization_to_nek'

  [csv]
    file_base = 'csv/solid_nek'
    type = CSV
  []
[]
