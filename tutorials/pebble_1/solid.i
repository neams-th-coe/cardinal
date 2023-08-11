pebble_diameter = 0.03
thermal_conductivity = 2.0

[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = ${fparse pebble_diameter / 2.0}
    nr = 2
  []
[]

[Variables]
  [temp]
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = BodyForce
    value = 15774023
    variable = temp
  []
[]

[BCs]
  [match_nek]
    type = MatchedValueBC
    variable = temp
    boundary = '0'
    v = 'nek_temp'
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '${thermal_conductivity}'
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  dt = 10.0
  nl_abs_tol = 1e-8

  steady_state_detection = true
  steady_state_tolerance = 1e-5
[]

[Outputs]
  exodus = true
  hide = 'flux_integral'
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
    sub_cycling = true
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppGeneralFieldNearestNodeTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestNodeTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
  []
  [flux_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
[]

[AuxVariables]
  [nek_temp]
  []
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    diffusion_variable = temp
    component = normal
    diffusivity = thermal_conductivity
    variable = flux
    boundary = '0'
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = temp
    boundary = '0'
  []
[]
