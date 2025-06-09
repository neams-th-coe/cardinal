channel_diameter = 0.016                 # diameter of the coolant channels (m)
n_bundles = 12                           # number of bundles in the full core
n_coolant_channels_per_block = 108       # number of coolant channels per assembly
unit_cell_height = 1.6                   # unit cell height - arbitrarily selected
kernel_radius = 214.85e-6                # fissile kernel outer radius (m)
buffer_radius = 314.85e-6                # buffer outer radius (m)
iPyC_radius = 354.85e-6                  # inner PyC outer radius (m)
SiC_radius = 389.85e-6                   # SiC outer radius (m)
oPyC_radius = 429.85e-6                  # outer PyC outer radius (m)
buffer_k = 0.5                           # buffer thermal conductivity (W/m/K)
PyC_k = 4.0                              # PyC thermal conductivity (W/m/K)
SiC_k = 13.9                             # SiC thermal conductivity (W/m/K)
kernel_k = 3.5                           # fissil kernel thermal conductivity (W/m/K)
matrix_k = 15.0                          # graphite matrix thermal conductivity (W/m/K)
inlet_T = 598.0                          # inlet fluid temperature (K)
power = 200e6                            # full core power (W)
mdot = 117.3                             # fluid mass flowrate (kg/s)
num_layers_for_plots = 50                # number of layers to average fields over for plotting
height = 6.343                           # height of the full core (m)
fluid_density = 5.5508                   # fluid density (kg/m3)
fluid_Cp = 5189.0                        # fluid isobaric specific heat (J/kg/K)
triso_pf = 0.15                          # TRISO packing fraction (%)

density_blocks = 'coolant'
temperature_blocks = 'graphite compacts compacts_trimmer_tri'
fuel_blocks = 'compacts compacts_trimmer_tri'

unit_cell_power = ${fparse power / (n_bundles * n_coolant_channels_per_block) * unit_cell_height / height}
unit_cell_mdot = ${fparse mdot / (n_bundles * n_coolant_channels_per_block)}

# compute the volume fraction of each TRISO layer in a TRISO particle
# for use in computing average thermophysical properties
kernel_fraction = ${fparse kernel_radius^3 / oPyC_radius^3}
buffer_fraction = ${fparse (buffer_radius^3 - kernel_radius^3) / oPyC_radius^3}
ipyc_fraction = ${fparse (iPyC_radius^3 - buffer_radius^3) / oPyC_radius^3}
sic_fraction = ${fparse (SiC_radius^3 - iPyC_radius^3) / oPyC_radius^3}
opyc_fraction = ${fparse (oPyC_radius^3 - SiC_radius^3) / oPyC_radius^3}

N = 50

U_ref = ${fparse mdot / (n_bundles * n_coolant_channels_per_block) / fluid_density / (pi * channel_diameter * channel_diameter / 4.0)}
t0 = ${fparse channel_diameter / U_ref}
nek_dt = 6e-3

[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_mesh_in.e
  []
[]

[Problem]
  type = FEProblem
  material_coverage_check = false
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
    block = ${temperature_blocks}
  []
  [source]
    type = CoupledForce
    variable = T
    v = power
    block = ${fuel_blocks}
  []
  [null] # The value of T on the fluid blocks is meaningless
    type = NullKernel
    variable = T
    block = ${density_blocks}
  []
[]

[BCs]
  [pin_outer]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = 'fluid_solid_interface'
  []
[]

[ICs]
  [nek_temp]
    type = FunctionIC
    variable = nek_temp
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
    block = ${fuel_blocks}
  []
[]

[AuxVariables]
  [nek_temp]
    block = ${density_blocks}
  []
  [nek_bulk_temp]
    block = ${density_blocks}
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
    block = ${fuel_blocks}
  []
  [max_block_T]
    type = ElementExtremeValue
    variable = T
    value_type = max
    block = 'graphite'
  []
  [synchronization_to_nek]
    type = Receiver
    default = 1.0
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = ${fuel_blocks}
    execute_on = transfer
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
    variable = nek_temp
    from_boundaries = '3'
    to_boundaries = 'fluid_solid_interface'
    from_multi_app = nek
  []
  [bulk_temperature_to_bison]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    variable = nek_bulk_temp
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
[]

[UserObjects]
  [average_fuel_axial]
    type = LayeredAverage
    variable = T
    direction = z
    num_layers = ${num_layers_for_plots}
    block = ${fuel_blocks}
  []
  [average_block_axial]
    type = LayeredAverage
    variable = T
    direction = z
    num_layers = ${num_layers_for_plots}
    block = 'graphite'
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

[Outputs]
  exodus = true
  hide = 'synchronization_to_nek'

  [csv]
    file_base = 'csv/solid_nek'
    type = CSV
  []
[]
