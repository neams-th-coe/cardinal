!include cell_tallies/openmc.i

[Functions]
  [turn_on_relaxation]
    type = ParsedFunction
    expression = 't > 2.0'
  []
[]

[Controls]
  [control_relaxation]
    type = BoolFunctionControl
    function = 'turn_on_relaxation'
    parameter = '*/*/allow_relaxation'
    # Execute before we run OpenMC
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Problem]
  allow_relaxation = false
  relaxation = 'constant'

  xml_directory = './cell_tallies'

  [Tallies/Cell]
    output = 'unrelaxed_tally'
  []
[]

[Outputs]
  exodus := false
  execute_on = 'TIMESTEP_END'
[]

[Postprocessors]
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
  # Gold against whether the tally values are the same or not.
  # If the relaxation control works, they will be identical until
  # timestep 3 (when relaxation is turned on).
  [p1_equal]
    type = ParsedPostprocessor
    expression = 'if(p1 = p1_raw, 1.0, 0.0)'
    pp_names = 'p1 p1_raw'
  []
  [p2_equal]
    type = ParsedPostprocessor
    expression = 'if(p2 = p2_raw, 1.0, 0.0)'
    pp_names = 'p2 p2_raw'
  []
  [p3_equal]
    type = ParsedPostprocessor
    expression = 'if(p3 = p3_raw, 1.0, 0.0)'
    pp_names = 'p3 p3_raw'
  []
[]

[Outputs]
  hide = 'heat_source p1 p2 p3 p1_raw p2_raw p3_raw'
[]
