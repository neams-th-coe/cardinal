[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
#  displacements = 'disp_x disp_y disp_z'
  moving_mesh = true
[]

[Problem]
  type = NekRSProblem
  casename = 'nekbox'
[]

[AuxVariables]
  [dummy]
  []
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
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
  [Quadrature]
    type = GAUSS_LOBATTO
    order = SECOND
  []
[]
