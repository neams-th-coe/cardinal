[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

# We just want to check that Cardinal can run Sockeye as a sub-app.
# We omit all transfers just to check that the code executes.
[MultiApps]
  [sockeye]
    type = TransientMultiApp
    app_type = SockeyeApp
    input_files = 'flow_only_convection.i'
    execute_on = timestep_end
  []
[]
