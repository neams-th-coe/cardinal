[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
  [average_temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [nek_temp]
  []
  [avg_flux]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
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
  [outside]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 300
  []
[]

[AuxKernels]
  [average_temp]
    variable = average_temp
    type = SpatialUserObjectAux
    user_object = average_temp
    execute_on = 'initial timestep_end'
  []
  [avg_flux]
    type = DiffusionFluxAux
    diffusion_variable = temp
    component = normal
    diffusivity = thermal_conductivity
    variable = avg_flux
    boundary = '1'
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
  num_steps = 100
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
  nl_rel_tol = 1e-5
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [./average_temp]
    type = NearestPointAverage
    variable = temp
    points = '0 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    app_type = OpenMCApp
    input_files = 'openmc.i'
    execute_on = timestep_begin
  []
  [nek]
    type = TransientMultiApp
    app_type = NekApp
    input_files = 'nek.i'
  []
[]

[Transfers]
  [./heat_source_from_openmc]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    multi_app = openmc
    source_variable = heat_source
    variable = heat_source
  []
  [./average_temp_to_openmc]
    type = NearestPointReceiverTransfer
    direction = to_multiapp
    multi_app = openmc
    from_uo = average_temp
    to_uo = average_temp
  []
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
  [total_flux_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = total_flux
    direction = to_multiapp
    from_postprocessor = total_flux
    multi_app = nek
  []
[]

[Postprocessors]
  [total_flux]
    type = SideFluxIntegral
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '1'
  []
  [average_flux]
    type = SideFluxAverage
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '1'
  []
[]
