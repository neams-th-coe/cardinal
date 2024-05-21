################################################################################
## KRUSTY                                                                     ##
## Heat Pipe Microreactor Steady State                                        ##
##                                                                            ##
## Mahmoud (03/25) (openmc.i/solid.i/model03.py/krusty_scale.e)                ##
################################################################################

[GlobalParams]
  flux_conversion_factor =1
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = krusty_scale.e
  []
  # [scale]
  #  type = TransformGenerator
  #  transform = SCALE
  #  vector_value = '0.01 0.01 0.01'
  #  input = fmg
  # []
[]

[Variables]
  [T]
    initial_condition = 1073
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
  [heat_source_fuel]
    type = CoupledForce
    variable = T
    v = power
    block = 'core'
  []
[]


[BCs]
  # [hp_temp]
  #  type = DirichletBC 
  #  variable = T
  #  boundary = '1 2 3 4'
  #  value = 1073
  # []
  [hp_temp]
    type = MatchedValueBC 
    variable = T
    boundary = '1'
    v = hp_temp_aux 
  []
  [outside]
    type = CoupledConvectiveHeatFluxBC 
    variable = T
    boundary = '5'
    htc = 5e4 # W/K/m^2
    T_infinity = 343 
  []
[]

[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 5
    block = 'core'
  []
  [fuel_thermal_conductivity]
    block = 'core'
    order = CONSTANT
    family = MONOMIAL
  []
  [hp_temp_aux]
    block = 'core'
    initial_condition = 1073
  []
  [layered_average]
    order = CONSTANT
    family = MONOMIAL
  []
  [flux_uo] #auxvariable to hold heat pipe surface flux from UserObject
    block = 'core'
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [fuel_thermal_conductivity]
    type = MaterialRealAux
    variable = fuel_thermal_conductivity
    property = thermal_conductivity
    execute_on = timestep_end
  []
  [layered_average]
    type = SpatialUserObjectAux
    variable = layered_average
    execute_on = timestep_end
    user_object = axial_plot
  []
  [flux_aux]
    type = DiffusionFluxAux
    component = normal
    diffusion_variable = T
    diffusivity = 'thermal_conductivity'
    variable = flux_uo
    execute_on = linear
    boundary = 1
  []
[]

[Materials]
  [fuel_thermal]
    type = HeatConductionMaterial
    block = 'core'
    temp = T
    thermal_conductivity = 38.65 # W/m/K
  []
  [helium_thermal]
    type = HeatConductionMaterial
    block = 'cavity_center gap_clamp gap_ref gap_sleeve gap_vaccan'
    temp = T
    thermal_conductivity = 1e-1 # W/m/K
  []
  [reflector_thermal]
    type = HeatConductionMaterial
    block = 'ref_bottom ref_top'
    temp = T
    thermal_conductivity = 325 # W/m/K
  []
  [MLI_thermal]
    type = HeatConductionMaterial
    block = 'MLI'
    temp = T
    thermal_conductivity = 138 # W/m/K
  []
  [SS_thermal]
    type = HeatConductionMaterial
    block = 'clamp sleeve vacuum_can'
    temp = T
    thermal_conductivity = 16.3 # W/m/K
  []
[]

[MultiApps]
  [sockeye]
     type = TransientMultiApp
     app_type = SockeyeApp
     positions_file = 'hp_centers.txt'
     input_files = 'sockeye.i'
     execute_on = 'timestep_begin' 
     max_procs_per_app = 1
     output_in_position = true
     library_path = '/apps/herd/sockeye/sockeye-2024-02-27/lib/sockeye'
     sub_cycling = true
  []
[]

[Transfers]
  [from_sockeye_temp]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sockeye
    source_variable = hp_temp_aux
    variable = hp_temp_aux
    execute_on = 'timestep_begin'
    to_boundaries = '1'
  []
  [to_sockeye_flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sockeye
    source_variable = flux_uo
    variable = master_flux
    execute_on = 'timestep_begin'
  []
  #    [to_sockeye_flux]
  #      type = MultiAppGeneralFieldNearestLocationTransfer
  #      to_multi_app = sockeye
  #      source_user_object = flux_UO
  #      variable = master_flux
  #      execute_on = 'timestep_begin'
  #    []
[]

[UserObjects]
  [axial_plot]
    type = NearestPointLayeredAverage
    variable = power
    num_layers = 16
    direction = z
    block = 'core'
    points = '0.0 0.0 0.0'
    direction_min = 0.1249
    direction_max = 0.3749
  []
  # [flux_uo]
  #  type = NearestPointLayeredSideDiffusiveFluxAverage
  #  direction = z
  #  num_layers = 100
  #  points_file = 'hp_centers.txt'
  #  variable = T
  #  diffusivity = 'thermal_conductivity'
  #  execute_on = linear
  #  boundary = '1'
  # []
  [flux_UO]
    type = NearestPointLayeredSideAverage
    direction = z
    num_layers = 100
    points_file = 'hp_centers.txt'
    variable = flux_uo
    execute_on = linear
    boundary = '1'
  []
[]

[Executioner]
  type = Transient
  dt = 1000

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-5

[]

[Postprocessors]
  [hp_heat_integral]
    type = SideDiffusiveFluxIntegral
    variable = T
    boundary = '1'
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  []
  [fuel_temp_avg]
    type = ElementAverageValue
    variable = T
    block = 'core'
  []
  [fuel_temp_max]
    type = ElementExtremeValue
    variable = T
    block = 'core'
  []
  [fuel_temp_min]
    type = ElementExtremeValue
    variable = T
    block = 'core'
    value_type = min
  []
  [heatpipe_surface_temp_avg]
    type = SideAverageValue
    variable = T
    boundary = 1
  []
  [power_integral]
    type = ElementIntegralVariablePostprocessor
    block = 'core'
    variable = power
    execute_on = 'timestep_end'
   []
  [fuel_k]
    type = ElementExtremeValue
    value_type = 'max'
    variable = fuel_thermal_conductivity
    block = 'core'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  perf_graph = true
  exodus = true
  color = true
  csv = true
  checkpoint = true
[]
