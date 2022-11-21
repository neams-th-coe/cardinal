[Problem]
  type = NekRSStandaloneProblem
  casename = 'pyramid'

  nondimensional = true
  U_ref = 0.1
  L_ref = 1.0
  rho_0 = 1.0
  Cp_0 = 1.0
[]

[Mesh]
  type = NekRSMesh
  scaling = 1.0
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  [out]
    type = CSV
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'initial timestep_begin'
  []
[]
