[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'turbPipe'
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = ChickadeeApp
    input_files = chickadee_sub.i
    execute_on = timestep_end

    # change this for wherever you have Chickadee located
    library_path = '/home/anovak/Chickadee/lib'
  []
[]

[Transfers]
  # fill this out with whatever data transfers you want to/from Chickadee
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
