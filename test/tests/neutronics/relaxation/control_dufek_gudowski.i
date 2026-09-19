!include cell_tallies/openmc.i

[Functions]
  [turn_on_relaxation]
    type = ParsedFunction
    expression = '((t >= 2.0) & (t < 4.0)) | (t > 5.0)'
  []
[]

[Controls]
  [control_relaxation]
    type = BoolFunctionControl
    function = 'turn_on_relaxation'
    parameter = '*/*/relaxation_on'
    # Execute before we run OpenMC
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Problem]
  relaxation = 'dufek_gudowski'
  first_iteration_particles = 1000

  xml_directory = './cell_tallies'

  [Tallies/Cell]
    output = 'unrelaxed_tally'
  []
[]

[Executioner]
  num_steps := 6
[]

[Outputs]
  exodus := false
  execute_on = 'TIMESTEP_END'
[]

[Postprocessors]
  [p1_prev]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 0.02'
    execute_on = 'TIMESTEP_BEGIN'
  []
  [p2_prev]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 0.06'
    execute_on = 'TIMESTEP_BEGIN'
  []
  [p3_prev]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 0.10'
    execute_on = 'TIMESTEP_BEGIN'
  []

  [p1_raw]
    type = PointValue
    variable = kappa_fission_raw
    point = '0.0 0.0 0.02'
  []
  [p2_raw]
    type = PointValue
    variable = kappa_fission_raw
    point = '0.0 0.0 0.06'
  []
  [p3_raw]
    type = PointValue
    variable = kappa_fission_raw
    point = '0.0 0.0 0.10'
  []

  # Compute the values of alpha. These are deterministic based on the number
  # of iterations and particles run with relaxation. The first value of alpha
  # is 1.0 (iteration 2), the second value of alpha is ~0.6180 (iteration 3),
  # and the final value of alpha is ~0.4558 (iteration 6). Otherwise, alpha should
  # be 1.0 as relaxation is disabled.
  [p1_alpha]
    type = ParsedPostprocessor
    expression = '(p1 - p1_prev) / (p1_raw - p1_prev)'
    pp_names = 'p1 p1_prev p1_raw'
  []
  [p2_alpha]
    type = ParsedPostprocessor
    expression = '(p2 - p2_prev) / (p2_raw - p2_prev)'
    pp_names = 'p2 p2_prev p2_raw'
  []
  [p3_alpha]
    type = ParsedPostprocessor
    expression = '(p3 - p3_prev) / (p3_raw - p3_prev)'
    pp_names = 'p3 p3_prev p3_raw'
  []

  [particles]
    type = OpenMCParticles
    value_type = instantaneous
  []
[]

[Outputs]
  hide = 'heat_source p1 p1_prev p1_raw p2 p2_prev p2_raw p3 p3_prev p3_raw'
[]
