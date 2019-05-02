[Mesh]
  type = FileMesh
  file = sphere.e
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
    type = HeatSource
    variable = temp
    function = 'openmc_heat_func'
  []
[]

[Functions]
  [openmc_heat_func]
    type = ParsedFunction
    value = 'openmc_heat_source'
    vars = 'openmc_heat_source'
    vals = 'openmc_heat_source'
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

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '20' # W/mK
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 1000
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
  nl_rel_tol = 1e-5
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    app_type = OpenMCApp
    input_files = 'openmc.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [./heat_source_from_openmc]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = openmc
    reduction_type = average
    from_postprocessor = heat_source
    to_postprocessor = openmc_heat_source
  []
[]

[Postprocessors]
  [./openmc_heat_source]
    type = Receiver
  []
[]
