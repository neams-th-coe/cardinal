# copy-pasta from common.i
inlet_T = 598.0                          # inlet fluid temperature (K)
mdot = ${fparse 117.3 / 12 / 108}        # fluid mass flowrate (kg/s)
outlet_P = 7.1e6                         # fluid outlet pressure (Pa)
channel_diameter = 0.016                 # diameter of the coolant channels (m)
height = 6.343                           # height of the assembly (m)

num_layers_for_THM = 50                  # number of elements in the THM model; for the converged case,
                                         # we set this to 150

[GlobalParams]
  initial_p = ${outlet_P}
  initial_T = ${inlet_T}
  initial_vel = ${fparse mdot / outlet_P / 8.3144598 * 4.0e-3 / inlet_T / (pi * channel_diameter * channel_diameter / 4.0)}

  rdg_slope_reconstruction = full
  closures = none
  fp = helium
[]

[Closures]
  [none]
    type = Closures1PhaseNone
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
    expression = '0.022 * pow(Re, 0.8) * pow(Pr, 0.4)'
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
    position = '0 0 ${height}'
    orientation = '0 0 -1'

    A = ${fparse pi * channel_diameter * channel_diameter / 4}
    D_h = ${channel_diameter}
    length = ${height}
    n_elems = ${num_layers_for_THM}
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'channel:in'
    m_dot = ${mdot}
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

  [console]
    type = Console
    outlier_variable_norms = false
  []
[]
