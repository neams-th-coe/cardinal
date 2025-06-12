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
  [source] # the "units" of a kernel in the heat equation are W/m^3, so we need to divide the power by the pellet volumes
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
    gap_conductivity = 60.0

    # geometric terms related to the gap
    gap_geometry_type = CYLINDER
    cylinder_axis_point_1 = '0 0 0'
    cylinder_axis_point_2 = '0 0 ${L}'
  []
[]

[BCs]
  [pin_and_duct]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = '5 10'
  []
[]

[Materials]
  [clad_and_duct]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '26'
    block = '1 10'
  []
  [pellet]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '23'
    block = '2 3'
  []
[]

[Postprocessors]
  [flux_integral] # evaluate the total heat flux for normalization
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = '5 10'
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
    input_files = 'nek.i'
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
    to_boundaries = '5 10'
  []
  [flux] # sends heat flux in flux to nekRS
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = heat_flux
    from_boundaries = '5 10'
  []
  [flux_integral_to_nek] # sends the heat flux integral (for normalization) to nekRS
    type = MultiAppPostprocessorTransfer
    to_postprocessor = heat_flux_integral
    from_postprocessor = flux_integral
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
    block = '1 10'
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
