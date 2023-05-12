# copy-pasta from common_input.i
inlet_T = 598.0                          # inlet fluid temperature (K)
buffer_k = 0.5                           # buffer thermal conductivity (W/m/K)
PyC_k = 4.0                              # PyC thermal conductivity (W/m/K)
SiC_k = 13.9                             # SiC thermal conductivity (W/m/K)
kernel_k = 3.5                           # fissil kernel thermal conductivity (W/m/K)
matrix_k = 15.0                          # graphite matrix thermal conductivity (W/m/K)
num_layers_for_plots = 50                # number of layers to average fields over for plotting
triso_pf = 0.15                          # TRISO packing fraction (%)
kernel_radius = 214.85e-6                # fissile kernel outer radius (m)
buffer_radius = 314.85e-6                # buffer outer radius (m)
iPyC_radius = 354.85e-6                  # inner PyC outer radius (m)
SiC_radius = 389.85e-6                   # SiC outer radius (m)
oPyC_radius = 429.85e-6                  # outer PyC outer radius (m)

# compute the volume fraction of each TRISO layer in a TRISO particle
# for use in computing average thermophysical properties
kernel_fraction = ${fparse kernel_radius^3 / oPyC_radius^3}
buffer_fraction = ${fparse (buffer_radius^3 - kernel_radius^3) / oPyC_radius^3}
ipyc_fraction = ${fparse (iPyC_radius^3 - buffer_radius^3) / oPyC_radius^3}
sic_fraction = ${fparse (SiC_radius^3 - iPyC_radius^3) / oPyC_radius^3}
opyc_fraction = ${fparse (oPyC_radius^3 - SiC_radius^3) / oPyC_radius^3}

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
    vars = 'k_TRISO k_graphite'
    vals = 'k_TRISO k_graphite'
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
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = 'compacts'
    execute_on = 'transfer'
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

[Executioner]
  type = Transient
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
    block = 'compacts'
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
  print_linear_residuals = false

  [csv]
    type = CSV
    file_base = 'csv_thm/solid_out'
  []
[]
