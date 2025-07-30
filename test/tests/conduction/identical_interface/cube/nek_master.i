# This mesh is the left half of the domain - it is simple enough that we can generate it
# here without requiring a separate exodus file.
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
    initial_condition = 500.0
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
    type = MatchedValueBC
    variable = temperature
    v = nek_temp
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
  num_steps = 30
  dt = 0.1
  nl_abs_tol = 1e-8
  #steady_state_detection = true
  #steady_state_tolerance = 1e-4
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
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
    source_boundary = 'right'
  []
  [flux_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [nek_temp]
    # Because we run nekRS _after_ MOOSE, for the very first time step, the
    # MatchedValueBC would try to set a zero-temperature condition at the interface
    # if we did not set an initial condition on nek_temp here. This is an arbitrary
    # value - we choose 550 (halfway between the Dirichlet BCs at the ends of the
    # rectangular prism) as a good guess of the interface temperature.
    #
    # One very important point that arises for this case is that if you were to choose
    # an initial condition of 500 (say, to match the initial condition set on the
    # 'temperature' variable), then your Master app solve would be fully-insulated BCs,
    # so you waste ~10 multiapp iterations just inching the interface flux up from a very
    # tiny initial value.
    initial_condition = 550.0
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temperature
    component = normal
    diffusivity = thermal_conductivity
    boundary = 'right'
  []
[]

[Functions]
  [bison_analytic]
    type = ParsedFunction
    expression = '125.0*x+562.5'
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
  [flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'left'
  []
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = 'right'
  []

  # Because the boundary conditions in the y- and z-directions are insulated, we
  # can compute the (constant) temperature on the interface at a single point
  [bison_temp_interface]
    type = PointValue
    variable = temperature
    point = '0.0 0.0 0.0'
  []
  [nek_temp_interface]
    type = PointValue
    variable = nek_temp
    point = '0.0 0.0 0.0'
  []
  [bison_l2_error]
    type = ElementL2Error
    variable = temperature
    function = bison_analytic
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  print_linear_residuals = false
[]
