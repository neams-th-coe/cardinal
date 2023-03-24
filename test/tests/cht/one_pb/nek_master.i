
[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'sphere.e'
  []

  [scale01]
    type = TransformGenerator
    input = 'fmg'
    transform = SCALE 
    vector_value = '0.2666 0.2666 0.2666' # r_inner_shell = 0.4 -> scaled from 1.5 in sphere.e
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [hc_time]
    type = HeatCapacityConductionTimeDerivative
    variable = temp
  []
  [heat]
    type = BodyForce
    value = 6.0
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
    prop_values = '0.0244' # Triso(26 W/mK)/He(0.15 W/mK) conductivities * Nek (1/7100)
    prop_names = 'thermal_conductivity'
  []
  [hc_time]
    type = GenericConstantMaterial
    prop_values =  '0.3267' #Currently set to nek fluid cp * ratio between graphite (1697 J/kg*K) and helium cp (5193 J/kgK)
    prop_names = 'heat_capacity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 10000
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-3
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-10
[]

[Variables]
  [temp]
    initial_condition = 100
  []
[]

[Outputs]
  exodus = true
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
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    direction = from_multiapp
    multi_app = nek
    variable = nek_temp
    fixed_meshes = true
  []
  [avg_flux]
    type = MultiAppNearestNodeTransfer
    source_variable = avg_flux
    direction = to_multiapp
    multi_app = nek
    variable = avg_flux
    fixed_meshes = true
  []
  [flux_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    direction = to_multiapp
    from_postprocessor = flux_integral
    multi_app = nek
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
  [./Volume]
    type = VolumePostprocessor
    execute_on = 'initial'
  [../]
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
  [average_pebble_T]
    type = SideAverageValue
    variable = 'temp'
    boundary = '1'
  []
[]