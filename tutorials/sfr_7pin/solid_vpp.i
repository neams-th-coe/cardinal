d_pellet = 0.603e-2                           # pellet diameter (m)
d_pin = 8.0e-3                                # pin (clad) diameter (m)
t_clad = 0.52e-3                              # clad thickness (m)
L = 20.32e-2                                  # height (m)
bundle_pitch = 0.02625                        # flat-to-flat distance inside the duct (m)
duct_thickness = 0.004                        # duct thickness (m)
pin_power = 10e3                              # bundle power (kW)

!include mesh.i

[GlobalParams]
  search_value_conflicts = false
[]

[Variables]
  [T]
    initial_condition = 500.0
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = T
  []
  [source]
    type = BodyForce
    variable = T
    function = ${fparse pin_power / (pi * (d_pellet * d_pellet / 4.0) * L)}
    block = '2 3'
  []
[]

[ThermalContact]
  # This adds boundary conditions bewteen the fuel and the cladding, which represents
  # the heat flux in both directions as
  # q''= h * (T_1 - T_2)
  # where h is a conductance that accounts for conduction through a material and
  # radiation between two infinite parallel plate gray bodies.
  [one_to_two]
    type = GapHeatTransfer
    variable = T
    primary = '1'
    secondary = '4'

    # we will use a quadrature-based approach to find the gap width and cross-side temperature
    quadrature = true

    # emissivity of the fuel
    emissivity_primary = 0.8

    # emissivity of the clad
    emissivity_secondary = 0.8

    # thermal conductivity of the gap material
    gap_conductivity_function = k_sodium
    gap_conductivity_function_variable = T

    # geometric terms related to the gap
    gap_geometry_type = CYLINDER
    cylinder_axis_point_1 = '0 0 0'
    cylinder_axis_point_2 = '0 0 ${L}'
  []
[]

[BCs]
  [pin_outer]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = '5'
  []
  [duct_inner]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = '10'
  []
[]

[Functions]
  [k_sodium]
    type = ParsedFunction
    expression = '1.1045e2 + -6.5112e-2 * t + 1.5430e-5 * t * t + -2.4617e-9 * t * t * t'
  []
  [k_HT9]
    type = ParsedFunction
    expression = 'if (t < 1030, 17.622 + 2.428e-2 * t - 1.696e-5 * t * t,
                           12.027 + 1.218e-2 * t)'
  []
  [k_U]
    type = ParsedFunction
    expression = 'if (t < 255.4, 16.170,
                            if (t < 1173.2, (5.907e-6 * t * t + 1.591e-2 * t + 11.712),
                                             38.508))'
  []
[]

[Materials]
  [clad_and_duct]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = k_HT9
    temp = T
    block = '1 10'
  []
  [pellet]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = k_U
    temp = T
    block = '2 3'
  []
[]

[VectorPostprocessors]
  [flux]
    type = VectorOfPostprocessors
    postprocessors = 'pin_flux duct_flux'
  []
[]

[Postprocessors]
  [pin_flux]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = '5'
  []
  [duct_flux]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = '10'
  []
  [max_fuel_T]
    type = NodalExtremeValue
    variable = T
    value_type = max
    block = '2 3'
  []
  [max_clad_T]
    type = NodalExtremeValue
    variable = T
    value_type = max
    block = '1'
  []
  [max_duct_T]
    type = NodalExtremeValue
    variable = T
    value_type = max
    block = '10'
  []
  [synchronize]
    type = Receiver
    default = 1
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek_vpp.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [nek_temp] # grabs temperature from nekRS and stores it in nek_temp
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    from_multi_app = nek
    variable = nek_temp
  []
  [flux] # sends heat flux in flux to nekRS
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = heat_flux
  []
  [flux_integral_to_nek] # sends the heat flux integral (for normalization) to nekRS
    type = MultiAppReporterTransfer
    to_reporters = 'heat_flux_integral/value'
    from_reporters = 'flux/flux'
    to_multi_app = nek
  []
  [synchronize_in]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronize
    to_multi_app = nek
  []
[]

[AuxVariables]
  [nek_temp]
    initial_condition = 500.0
  []
  [flux]
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
    boundary = '5 10'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-2
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  steady_state_detection = true
  steady_state_tolerance = 1e-2
[]

[Outputs]
  exodus = true
[]
