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
  n_usrwrk_slots = 4

  [FieldTransfers]
    [heat_source]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = source_integral
    []
    [disp]
      type = NekMeshDeformation
      usrwrk_slot = '1 2 3'
      direction = to_nek
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
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
  hide = 'heat_source transfer_in source_integral temp_ansol'
[]

