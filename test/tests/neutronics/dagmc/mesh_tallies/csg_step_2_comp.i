[AuxVariables]
  [kappa_fission_csg]
    family = MONOMIAL
    order = CONSTANT
  []
  [kappa_fission_std_dev_csg]
    family = MONOMIAL
    order = CONSTANT
  []
  [in_three_sigma]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [k_csg]
    type = PiecewiseConstant
    data_file = csg_step_2_openmc_out.csv
    direction = left
    format = columns
    x_title = 'time'
    y_title = 'k'
  []
  [k_std_dev_csg]
    type = PiecewiseConstant
    data_file = csg_step_2_openmc_out.csv
    direction = left
    format = columns
    x_title = 'time'
    y_title = 'k_std_dev'
  []
[]

[AuxKernels]
  [kappa_fission_csg]
    type = SolutionAux
    variable = kappa_fission_csg
    solution = csg_ref
    from_variable = 'kappa_fission'
  []
  [kappa_fission_std_dev_csg]
    type = SolutionAux
    variable = kappa_fission_std_dev_csg
    solution = csg_ref
    from_variable = 'kappa_fission_std_dev'
  []
  [kappa_fission_in_three_sigma]
    type = ParsedAux
    variable = in_three_sigma
    coupled_variables = 'kappa_fission_csg kappa_fission_std_dev_csg kappa_fission kappa_fission_std_dev'
    expression = 'upper := max(kappa_fission_csg, kappa_fission);
                  upper_std_dev := if(kappa_fission_csg >= kappa_fission, kappa_fission_std_dev_csg, kappa_fission_csg);
                  lower := min(kappa_fission_csg, kappa_fission);
                  lower_std_dev := if(kappa_fission_csg < kappa_fission, kappa_fission_std_dev_csg, kappa_fission_csg);
                  if(upper - 3.0 * upper_std_dev <= lower + 3.0 * lower_std_dev, 1, 0)'
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [csg_ref]
    type = SolutionUserObject
    mesh = csg_step_2_openmc_out.e
    system_variables = 'kappa_fission kappa_fission_std_dev'
  []
[]

[Postprocessors]
  [k_csg_pp]
    type = FunctionValuePostprocessor
    function = k_csg
    point = '0.0 0.0 0.0'
    time = '2.0'
  []
  [k_std_dev_csg_pp]
    type = FunctionValuePostprocessor
    function = k_std_dev_csg
    point = '0.0 0.0 0.0'
    time = '2.0'
  []

  [k_in_three_sigma]
    type = ParsedPostprocessor
    pp_names = 'k_csg_pp k k_std_dev_csg_pp k_std_dev'
    expression = 'upper := max(k_csg_pp, k);
                  upper_std_dev := if(k_csg_pp >= k, k_std_dev_csg_pp, k_std_dev);
                  lower := min(k_csg_pp, k);
                  lower_std_dev := if(k_csg_pp < k, k_std_dev_csg_pp, k_std_dev);
                  if(upper - 3.0 * upper_std_dev <= lower + 3.0 * lower_std_dev, 1, 0)'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  hide = 'kappa_fission_csg kappa_fission
          kappa_fission_std_dev_csg kappa_fission_std_dev
          k k_std_dev k_csg_pp k_std_dev_csg_pp
          cell_instance'
[]
