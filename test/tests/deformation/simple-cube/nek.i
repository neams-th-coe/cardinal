[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  type = NekRSProblem
  casename = 'nekbox'
  synchronization_interval = parent_app
[]

[AuxVariables]
  [temp_ansol]
    order = SECOND
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [integral]
    type = ElementL2Difference
    variable = temp
    other_variable = temp_ansol
    execute_on = timestep_end
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
    execute_on = timestep_end
  []
  [min_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
    execute_on = timestep_end
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'

  # uncomment the temp_ansol to see that the solution matches very well
  hide = 'source_integral transfer_in heat_source temp_ansol'
[]

