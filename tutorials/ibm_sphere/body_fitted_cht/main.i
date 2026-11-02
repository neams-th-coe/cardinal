[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = 1.0
    nr = 4
  []
[]

sphere_volume = 4.171210

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = BodyForce
    function = 'q'
    variable = temp
  []
[]

[Functions]
  [q]
    type = ParsedFunction
    expression = 'if (y > 0, ${fparse 1*4*pi*1^2/sphere_volume}*2, 0.0)'
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
    prop_values = '0.1'
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 3000
  petsc_options_value = 'hypre boomeramg'
  dt = 0.02
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-10
[]

[Variables]
  [temp]
  []
[]

[Outputs]
  exodus = true
  hide = 'average_flux synchronize flux_integral'
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
  [synchronize_in]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronize
    to_multi_app = nek
  []
  [nek_temp]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
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
    variable = 'temp'
    boundary = '0'
  []
  [average_flux]
    type = SideDiffusiveFluxAverage
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '0'
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
  [synchronize]
    type = Receiver
    default = 1
  []
[]
