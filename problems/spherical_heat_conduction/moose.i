# This input runs heat conduction in the pebble, with data transfers to/from
# a sub-application that runs the fluid phase.

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
    type = CoupledForce
    variable = temp
    v = heat_source
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
  num_steps = 10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
[]

[Variables]
  [temp]
  []
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = NekApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = nek
    variable = nek_temp

    # this variable is defined in NekRSProblem, so it doesnt appear in this file
    source_variable = temp
  []
  [avg_flux]
    type = MultiAppNearestNodeTransfer
    source_variable = avg_flux
    direction = to_multiapp
    multi_app = nek

    # this variable is defined in NekRSProblem, so it doesnt appear in this file
    variable = avg_flux
  []
  [total_flux_to_nek]
    type = MultiAppPostprocessorTransfer
    direction = to_multiapp
    multi_app = nek
    from_postprocessor = total_flux

    # this variable is defined in NekRSProblem, so it doesnt appear in this file
    to_postprocessor = total_flux
  []
[]

[AuxVariables]
  [nek_temp] # received from nekRS
  []
  [avg_flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [heat_source] # received from OpenMC
    family = MONOMIAL
    order = CONSTANT
  []
  [average_temp] # this is strictly to visualize the temperature sent to OpenMC - not used in the transfer!
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
  [average_temp]
    variable = average_temp
    type = SpatialUserObjectAux
    user_object = average_temp
    execute_on = 'initial timestep_end'
  []
[]

[UserObjects]
  [./average_temp] # sent to OpenMC
    type = NearestPointAverage
    variable = temp
    points = '0 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Postprocessors]
  [total_flux]
    type = SideFluxIntegral
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
