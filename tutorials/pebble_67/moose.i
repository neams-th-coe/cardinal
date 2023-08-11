[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = 0.03
    nr = 2
  []
  [cmbn]
    type = CombinerGenerator
    inputs = sphere
    positions_file = 'pebble_positions.txt'
  []
[]

[Variables]
  [temp]
    initial_condition = 523.0
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat] # approximate value to get desired power
    type = BodyForce
    value = 338714.0
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
    prop_values = '5.0'
    prop_names = 'thermal_conductivity'
  []
[]

[AuxVariables]
  [nek_temp]
    initial_condition = 523.0
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

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
    fixed_meshes = true
  []
  [flux]
    type = MultiAppNearestNodeTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
    fixed_meshes = true
  []
  [flux_integral_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
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

t_nek = ${fparse 3.067e-01 * 2.0e-5}
M = 100.0

[Executioner]
  type = Transient
  num_steps = 10000
  dt = ${fparse M * t_nek}
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  hide = 'synchronize'
  interval = ${M}
[]

[Postprocessors]
  [flux_integral]
    type = SideDiffusiveFluxIntegral
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
  [volume]
    type = VolumePostprocessor
  []
  [synchronize]
    type = Receiver
    default = 1.0
  []
[]
