!include common_input.i

mdot_single_channel = ${fparse 117.3 / 12 / 108}        # individual coolant channel fluid mass flowrate (kg/s)

[GlobalParams]
  initial_p = ${outlet_P}
  initial_T = ${inlet_T}
  initial_vel = 1.0

  rdg_slope_reconstruction = full
  closures = none
  fp = helium
[]

[Closures]
  [none]
    type = WallTemperature1PhaseClosures
  []
[]

[FluidProperties]
  [helium]
    type = IdealGasFluidProperties
    molar_mass = 4e-3
    gamma = 1.668282 # should correspond to  Cp = 5189 J/kg/K
    k = 0.2556
    mu = 3.22639e-5
  []
[]

[AuxVariables]
  [T_wall]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Tw_aux]
    type = ADMaterialRealAux
    block = channel
    variable = T_wall
    property = T_wall
  []
[]

[Materials]
  # wall friction closure
  [f_mat]
    type = ADWallFrictionChurchillMaterial
    block = channel
    D_h = D_h
    f_D = f_D
    vel = vel
    rho = rho
    mu = mu
  []

  # Wall heat transfer closure (all important is in Nu_mat)
  [Re_mat]
    type = ADReynoldsNumberMaterial
    block = channel
    Re = Re
    D_h = D_h
    mu = mu
    vel = vel
    rho = rho
  []
  [Pr_mat]
    type = ADPrandtlNumberMaterial
    block = channel
    cp = cp
    mu = mu
    k = k
  []
  [Nu_mat]
    type = ADParsedMaterial
    block = channel
    # Dittus-Boelter
    expression = '0.023 * pow(Re, 0.8) * pow(Pr, 0.4)'
    property_name = 'Nu'
    material_property_names = 'Re Pr'
  []
  [Hw_mat]
    type = ADConvectiveHeatTransferCoefficientMaterial
    block = channel
    D_h = D_h
    Nu = Nu
    k = k
  []

  [T_wall]
    type = ADTemperatureWall3EqnMaterial
    Hw = Hw
    T = T
    q_wall = q_wall
  []
[]

[Components]
  [channel]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'

    A = ${fparse pi * channel_diameter * channel_diameter / 4}
    D_h = ${channel_diameter}
    length = ${height}
    n_elems = 50
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'channel:in'
    m_dot = ${mdot_single_channel}
    T = ${inlet_T}
  []

  [outlet]
    type = Outlet1Phase
    input = 'channel:out'
    p = ${outlet_P}
  []

  [ht_ext]
    type = HeatTransferFromExternalAppHeatFlux1Phase
    flow_channel = channel
    P_hf = ${fparse channel_diameter * pi}
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  start_time = 0

  # end_time = 1000
  steady_state_detection = true
  steady_state_tolerance = 1e-08

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  solve_type = NEWTON
  line_search = basic
[]

[Outputs]
  exodus = true
  print_linear_residuals = false

  [screen]
    type = Console
    outlier_variable_norms = false
    hide = 'max_p min_p'
  []

  [csv]
    file_base = 'csv/thm'
    type = CSV
  []
[]

[UserObjects]
  [average_fluid_axial]
    type = LayeredAverage
    variable = T
    direction = z
    num_layers = ${num_layers_for_plots}
  []
  [average_wall_axial]
    type = LayeredAverage
    variable = T_wall
    direction = z
    num_layers = ${num_layers_for_plots}
  []
  [average_pressure_axial]
    type = LayeredAverage
    variable = p
    direction = z
    num_layers = ${num_layers_for_plots}
  []
[]

[VectorPostprocessors]
  [fluid]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_fluid_axial
  []
  [fluid_wall]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_wall_axial
  []
  [pressure]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_pressure_axial
  []
[]

[Postprocessors]
  [max_T]
    type = ElementExtremeValue
    variable = T
  []
  [max_p]
    type = ElementExtremeValue
    variable = p
  []
  [min_p]
    type = ElementExtremeValue
    variable = p
    value_type = min
  []
  [dp]
    type = LinearCombinationPostprocessor
    pp_names = 'max_p min_p'
    pp_coefs = '1.0 -1.0'
  []
[]
