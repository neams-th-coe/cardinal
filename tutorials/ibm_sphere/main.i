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

  # in order to compare with the constant heat flux case, the body force we apply here leads to
  # a total power deposited into the fluid of q'' = 1, or 1 * 4 * pi * 1^2. However, the volume
  # of the sphere is not perfectly 4/3 * pi * 1^3, hence the adjustment here.
  [heat]
    type = BodyForce
    value = ${fparse 1*4*pi*1^2/sphere_volume}
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
    prop_values = '0.1'
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 100
  petsc_options_value = 'hypre boomeramg'
  dt = 1.0
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-10
[]

[Variables]
  [temp]
  []
[]

[Outputs]
  exodus = true
  #time_step_interval = 100
  hide = 'max_pebble_T min_pebble_T average_flux synchronize flux_integral'
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'cht.i'
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
    from_blocks = 9999
  []
  [avg_flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = avg_flux
    to_multi_app = nek
    variable = flux
    to_blocks = 9999
  []
  [flux_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral_stl
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
  [volume]
    type = VolumePostprocessor
  []
[]
