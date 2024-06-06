[GlobalParams]
    global_init_P = 1.0e5                       # Global initial fluid pressure
    global_init_V = 0.5                         # Global initial temperature for fluid and solid
    global_init_T = 628.15                      # Global initial fluid velocity
    scaling_factor_var = '1 1e-3 1e-6'          # Scaling factors for fluid variables (p, v, T)
[]

[EOS]
  [./eos]                                       # EOS name
    type = PBSodiumEquationOfState              # Using the sodium equation-of-state
  [../]
[]

[Functions]
  [./tin_sine]                                  # Function name
    type = ParsedFunction                       # Parsed function
    value = 628+100*sin(pi*t)                   # Parsed function formula
  [../]
[]

[Components]
  [./pipe1]
    type = PBOneDFluidComponent
    eos = eos                          # The equation-of-state name
    position = '0 0 0'                 # The origin position of this component
    orientation = '0 0 1'              # The orientation of the component
    heat_source = 0                    # Volumetric heat source
    f = 0.01                           # Specified friction coefficient
    Dh = 0.02                          # Equivalent hydraulic diameter
    length = 1                         # Length of the component
    n_elems = 100                      # Number of elements used in discretization
    A = 3.14e-4                        # Area of the One-D fluid component
  [../]

  [./inlet]
    type = PBTDJ
    input = 'pipe1(in)'                # Name of the connected components and the end type
    eos = eos                          # The equation-of-state
    v_bc = 0.5                         # Velocity boundary condition
    #T_bc = 628
    T_fn = tin_sine        # Temperature boundary condition
  [../]

  [./outlet]
    type = PBTDV
    input = 'pipe1(out) '              # Name of the connected components and the end type
    eos = eos                          # The equation-of-state
    p_bc = '1.0e5'                     # Pressure boundary condition
  [../]
[]

[Preconditioning]
  active = 'SMP_PJFNK'
  [./SMP_PJFNK]
    type = SMP                         # Single-Matrix Preconditioner
    full = true                        # Using the full set of couplings among all variables
    solve_type = 'PJFNK'               # Using Preconditioned JFNK solution method
    petsc_options_iname = '-pc_type'   # PETSc option, using preconditiong
    petsc_options_value = 'lu'         # PETSc option, using ‘LU’ precondition type in Krylov solve
  [../]
[] # End preconditioning block

[Postprocessors]
  [./Tin]
    type = ComponentBoundaryVariableValue
    variable = temperature
    input = pipe1(in)
  [../]
  [./Tout]
    type = ComponentBoundaryVariableValue
    variable = temperature
    input = pipe1(out)
  [../]
[]

[Executioner]
  type = Transient                    # This is a transient simulation

  dt = 0.02                           # Targeted time step size
  dtmin = 1e-5                        # The allowed minimum time step size

  petsc_options_iname = '-ksp_gmres_restart'  # Additional PETSc settings, name list
  petsc_options_value = '100'                 # Additional PETSc settings, value list

  nl_rel_tol = 1e-7                   # Relative nonlinear tolerance for each Newton solve
  nl_abs_tol = 1e-6                   # Relative nonlinear tolerance for each Newton solve
  nl_max_its = 20                     # Number of nonlinear iterations for each Newton solve

  l_tol = 1e-4                        # Relative linear tolerance for each Krylov solve
  l_max_its = 100                     # Number of linear iterations for each Krylov solve

  start_time = 0.0                    # Physical time at the beginning of the simulation
  num_steps = 20                     # Max. simulation time steps
  end_time = 10.                     # Max. physical time at the end of the simulation

  [./Quadrature]
    type = TRAP                       # Using trapezoid integration rule
    order = FIRST                     # Order of the quadrature
  [../]
[] # close Executioner section

[Outputs]
  perf_graph = true                      # Output the performance log
  csv = true
  [./console]
    type = Console                       # Screen output
  [../]
  [./out_displaced]
    type = Exodus                        # Output simulation data to an ExodusII file
    use_displaced = true                 # Use displaced mesh
    execute_on = 'initial timestep_end'  # Output data at the begining of the simulation and each time step
    sequence = false                     # Don't save sequential file output per time step
  [../]
[]
