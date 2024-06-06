# This input file was simply copied from Sockeye in order to get a Sockeye
# input - the physics are irrelevant, since we are only testing that Sockeye inputs
# can be run within Cardinal.

L_evap = 0.3
L_adia = 1.0
L_cond = 0.33

z_evap = 0
z_adia = ${fparse z_evap + L_evap}
z_cond = ${fparse z_adia + L_adia}

D_wick_i = 0.01231
D_wick_o = 0.01259
D_clad_i = 0.01410
R_pore   = 23.0e-6
porosity = 0.770621
permeability = 1e-9

P_clad_i = ${fparse pi * D_clad_i}

initial_T = 1200

[GlobalParams]
  # The parameters in this block are parameters of
  # the component class "HeatPipe3Phase". Putting them in this block will allow
  # the parameters to be substituted everywhere that these parameters exist.

  scaling_factor_arhoEA_lsw = 1e-6
  scaling_factor_arhoEA_vapor = 1e-6
  orientation = '0 0 1'
  gravity_vector = '0 0 0'

  D_clad_i = ${D_clad_i}
  D_wick_i = ${D_wick_i}
  D_wick_o = ${D_wick_o}
  R_pore = ${R_pore}
  porosity = ${porosity}
  permeability = ${permeability}

  k_solid = 100
  k_wick = 20
  cv_wick = 500
  rho_wick = 7900

  initial_T= ${initial_T}
  fill_ratio = 1.05

  fp = fp_2phase
  closures = closures

  rdg_slope_reconstruction = none
[]

[FluidProperties]
  [fp_2phase]
    type = SodiumTwoPhaseFluidProperties
  []
[]

[Closures]
  [closures]
    type = Closures3Phase
    htc_int_lsw = 1e5
    htc_int_vapor = 1e5
  []
[]

[Components]
  [bottom]
    type = HeatPipeEnd
    input = 'hp_evap:in'
  []

  [hp_evap]
    type = HeatPipe3Phase
    position = '0 0 ${z_evap}'
    length = ${L_evap}
    n_elems = 20
  []

  [junction_evap]
    type = Junction3Phase
    connections = 'hp_evap:out hp_adia:in'
  []

  [hp_adia]
    type = HeatPipe3Phase
    position = '0 0 ${z_adia}'
    length = ${L_adia}
    n_elems = 20
  []

  [junction_cond]
    type = Junction3Phase
    connections = 'hp_adia:out hp_cond:in'
  []

  [hp_cond]
    type = HeatPipe3Phase
    position = '0 0 ${z_cond}'
    length = ${L_cond}
    n_elems = 20
  []

  [top]
    type = HeatPipeEnd
    input = 'hp_cond:out'
  []

  [ht_evap]
    type = HTConvection3Phase
    flow_channel = hp_evap
    T_wall = 1200
    # If htc_wall_lsw and htc_wall_vapor are not specified, they are internally calculated
    # (see theory manual for correlation details)
    P_hf = ${P_clad_i}
  []

  [ht_adia]
    type = HTHeatFlux3Phase
    flow_channel = hp_adia
    q_wall = 0
  []

  [ht_cond]
    type = HTConvection3Phase
    flow_channel = hp_cond
    T_wall = 900
    htc_wall_lsw = 1000
    htc_wall_vapor = 0
    P_hf = ${P_clad_i}
  []
[]

[Postprocessors]
  # These post-processors integrates the heat flux on the evaporator and condenser regions.
  # The HeatRateConvection3Phase post-processor is used where the convective heat transfer has been prescribed.
  # The HeatRateDirectFlowChannel post-processor is used where the heat flux has been prescribed.
  # They are summed to verify the net power is zero which will allow a steady-state solution.
  [power_evap]
    type = HeatRateConvection3Phase
    T_wall = T_wall
    P_hf = P_hf
    block = 'hp_evap'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [power_adia]
    type = ADHeatRateDirectFlowChannel
    q_wall_prop = q_wall
    P_hf = ${P_clad_i}
    block = 'hp_adia'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [power_cond]
    type = HeatRateConvection3Phase
    T_wall = T_wall
    P_hf = P_hf
    block = 'hp_cond'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [net_power]
    type = SumPostprocessor
    values = 'power_evap power_adia power_cond'
    execute_on = 'INITIAL TIMESTEP_END'
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
  scheme = bdf2

  dt = 0.1

  steady_state_detection = true
  steady_state_tolerance = 1e-2

  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  # print_linear_converged_reason = false
  # print_nonlinear_converged_reason = false
  # print_linear_residuals = false

  file_base = 'flow_only_convection'
  [exodus]
    type = Exodus
    output_material_properties = true
    show_material_properties = 'alpha_vapor p_vapor p_liquid alpha_vapor_0 alpha_vapor_wick_i_max T_lsw T_vapor'
  []
[]

# We just want to check that Cardinal can run Sockeye as a master-app with Cardinal as a sub-app.
# We omit all transfers just to check that the code executes.
[MultiApps]
  [cardinal]
    type = TransientMultiApp
    input_files = 'cardinal_sub.i'
    execute_on = timestep_end
  []
[]
