!include common.i

flow_area = ${fparse (0.01 * PITCH * 0.01 * PITCH - pi * (0.01 * (R_FUEL + T_F_C_GAP + T_ZR_CLAD))^2)}
mass_flux_in = ${fparse M_DOT / flow_area}

[QuadSubChannelMesh]
  [assembly]
    type = SCMQuadAssemblyMeshGenerator
    # number of channels in the x-direction (number of pins in x plus 1)
    nx = 2
    # number of channels in the y-direction (number of pins in y plus 1)
    ny = 2

    # number of cells in the vertical direction
    n_cells = ${AXIAL_LAYERS}

    pitch = ${fparse 0.01 * PITCH}
    pin_diameter = ${fparse 0.01 * 2.0 * (R_FUEL + T_F_C_GAP + T_ZR_CLAD)}
    side_gap = 0.0
    heated_length = ${fparse 0.01 * HEIGHT}

    # do not model any grid spacers
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  n_blocks = 1
  fp = water
  P_tol = 1e-6
  T_tol = 1e-5
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_OUTLET}
  implicit = true
  segregated = false
  friction_closure = 'MATRA'
  pin_HTC_closure = 'Dittus-Boelter'
  full_output = true
  mixing_closure ='constant_beta'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 2.6
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_INLET}
  []
  [Dpin_IC]
    type = ConstantIC
    variable = Dpin
    value = ${fparse 0.01 * 2.0 * (R_FUEL + T_F_C_GAP + T_ZR_CLAD)}
  []
  [q_prime_IC]
    type = ConstantIC
    variable = q_prime
    value = ${fparse P_PIN / (0.01 * HEIGHT)}
  []
  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_OUTLET}
    T = T
    fp = water
  []
  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_OUTLET}
    T = T
    fp = water
  []
  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_OUTLET}
    T = T
    fp = water
  []
[]

[AuxKernels]
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_INLET}
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Steady
[]
