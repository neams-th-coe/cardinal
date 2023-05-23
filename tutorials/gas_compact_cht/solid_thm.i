# This input models conjugate heat transfer between MOOSE and THM for
# a single coolant channel. This input file should be run with:
#
# cardinal-opt -i common_input.i solid_thm.i

unit_cell_mdot = ${fparse mdot / (n_bundles * n_coolant_channels_per_block)}
unit_cell_power = ${fparse power / (n_bundles * n_coolant_channels_per_block) * unit_cell_height / height}

# compute the volume fraction of each TRISO layer in a TRISO particle
# for use in computing average thermophysical properties
kernel_fraction = ${fparse kernel_radius^3 / oPyC_radius^3}
buffer_fraction = ${fparse (buffer_radius^3 - kernel_radius^3) / oPyC_radius^3}
ipyc_fraction = ${fparse (iPyC_radius^3 - buffer_radius^3) / oPyC_radius^3}
sic_fraction = ${fparse (SiC_radius^3 - iPyC_radius^3) / oPyC_radius^3}
opyc_fraction = ${fparse (oPyC_radius^3 - SiC_radius^3) / oPyC_radius^3}

# multiplicative factor on assumed heat source distribution to get correct magnitude
q0 = ${fparse unit_cell_power / (4.0 * unit_cell_height * compact_diameter * compact_diameter / 4.0)}

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
    thermal_conductivity_temperature_function = k_TRISO
    temp = T
    block = 'compacts'
  []
[]

[Postprocessors]
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

  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = 'compacts'
  []
[]

[MultiApps]
  [thm]
    type = FullSolveMultiApp
    input_files = 'thm.i'
    execute_on = timestep_end
    max_procs_per_app = 1
    bounding_box_padding = '0.1 0.1 0'
  []
[]

[Transfers]
  [q_wall_to_thm]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = q_wall
    to_multi_app = thm
    source_user_object = q_wall_avg
  []
  [T_wall_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = T_wall
    from_multi_app = thm
    variable = fluid_temp
    fixed_meshes = true
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

[Executioner]
  type = Transient
  dt = 5e-3

  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'

  steady_state_detection = true
  steady_state_tolerance = 1e-03
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
  [q_wall_avg]
    type = LayeredSideDiffusiveFluxAverage
    boundary = fluid_solid_interface
    direction = z
    variable = T
    diffusivity = thermal_conductivity
    # Note: make this to match the num_elems in the channel
    num_layers = 50
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
  print_linear_residuals = false
  perf_graph = true

  [csv]
    type = CSV
    file_base = 'csv/solid_thm'
  []
[]
