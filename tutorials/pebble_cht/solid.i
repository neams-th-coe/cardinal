pebble_diameter = 0.03
thermal_conductivity = 2.0

[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = ${fparse pebble_diameter / 2.0}
    nr = 3
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
    v = nek_temp
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

  dt = 5.0
  nl_abs_tol = 1e-8

  steady_state_detection = true
  steady_state_tolerance = 1e-4
[]

[Outputs]
  csv = true
  exodus = true
  hide = 'flux_integral flux_nodal'
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux_nodal
    to_multi_app = nek
    variable = flux
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
  [flux_nodal]
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
  [flux_nodal]
    type = ProjectionAux
    variable = flux_nodal
    v = flux
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = temp
    boundary = '0'
  []
  [max_T]
    type = NodalExtremeValue
    variable = temp
  []
[]

[UserObjects]
  [average_flux_axial]
    type = LayeredSideAverage
    variable = flux
    direction = z
    num_layers = 5
    boundary = '0'
  []
[]

[VectorPostprocessors]
  [flux_axial]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_flux_axial
  []
[]
