[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = BodyForce
    value = 35.37
    variable = temp
  []
[]

[BCs]
  [match_nek]
    type = MatchedValueBC
    variable = temp
    boundary = '1'
    v = 'nek_temp'
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '0.2' # 20 W/mK -> 0.2 W/cmK
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 50
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-10
[]

[Variables]
  [temp]
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  hide = 'max_pebble_T min_pebble_T'
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [avg_flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = avg_flux
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
  [avg_flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [avg_flux]
    type = DiffusionFluxAux
    diffusion_variable = temp
    component = normal
    diffusivity = thermal_conductivity
    variable = avg_flux
    boundary = '1'
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '1'
  []
  [average_flux]
    type = SideDiffusiveFluxAverage
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '1'
  []
  [max_pebble_T]
    type = NodalExtremeValue
    variable = temp
    value_type = max
  []
  [min_pebble_T]
    type = NodalExtremeValue
    variable = temp
    value_type = min
  []
[]
