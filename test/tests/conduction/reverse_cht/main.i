[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 48
  ny = 5
  nz = 5
  xmin = -0.5
  xmax = 0.0
  ymin = -0.025
  ymax = 0.025
  zmin = -0.025
  zmax = 0.025
[]

[Variables]
  [temperature]
    initial_condition = 600.0
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = temperature
  []
  [diffusion]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = thermal_conductivity
  []
[]

[BCs]
  [interface]
    type = CoupledVarNeumannBC
    variable = temperature
    v = nek_flux
    boundary = 'right'
  []
  [vol2_top]
    type = DirichletBC
    variable = temperature
    boundary = 'left'
    value = 500.0
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  nl_abs_tol = 1e-8

  # relatively coarse convergence just to get a faster-running test
  steady_state_detection = true
  steady_state_tolerance = 1e-2
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_begin
  []
[]

[Transfers]
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    from_multi_app = nek
    variable = nek_flux
    search_value_conflicts = false
    source_boundary = 'right'
    from_postprocessors_to_preserved = flux_integral
    to_postprocessors_to_be_preserved = flux_integral
  []
  [temp]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    to_multi_app = nek
    variable = temperature
    source_boundary = 'right'
    search_value_conflicts = false
  []
[]

[AuxVariables]
  [nek_flux]
  []
[]

[Functions]
  [analytic]
    type = ParsedFunction
    expression = '100*(x+0.5)+500.0'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = nek_flux
    boundary = 'right'
  []

  # Because the boundary conditions in the y- and z-directions are insulated, we
  # can compute the (constant) temperature on the interface at a single point
  [temp_interface]
    type = PointValue
    variable = temperature
    point = '0.0 0.0 0.0'
  []
  [nek_flux_interface]
    type = PointValue
    variable = nek_flux
    point = '0.0 0.0 0.0'
  []
  [l2_error]
    type = ElementL2Error
    variable = temperature
    function = analytic
  []
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'final'
  print_linear_residuals = false
[]
