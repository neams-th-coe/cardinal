[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
[]

[Problem]
  type = NekRSProblem
  moving_mesh = true
[]

[AuxVariables]
  [temp_ansol]
    order = SECOND
  []
  [dummy]
  []
[]

# This AuxVariable and AuxKernel is only here to get the postprocessors
# to evaluate correctly. This can be deleted after MOOSE issue #17534 is fixed.
[AuxKernels]
  [dummy]
    type = ConstantAux
    variable = dummy
    value = 0.0
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
  hide = 'source_integral transfer_in heat_source dummy temp_ansol'
[]

