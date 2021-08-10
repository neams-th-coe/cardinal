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

# We just want to check that Cardinal can run THM as a master-app with Cardinal as a sub-app.
# We omit all transfers just to check that the code executes.
[MultiApps]
  [thm]
    type = TransientMultiApp
    app_type = THMApp
    input_files = 'sod_shock_tube.i'
    execute_on = timestep_end

    # THM is currently limited to serial execution
    max_procs_per_app = 1
  []
[]

[Outputs]
  hide = 'flux_integral'
[]
