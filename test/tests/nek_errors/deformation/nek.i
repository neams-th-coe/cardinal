[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  casename = 'user'
  has_heat_source = false
[]

[AuxVariables]
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
