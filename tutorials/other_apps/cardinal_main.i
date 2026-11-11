!include openmc.i

[MultiApps]
  [tmap8]
    type = FullSolveMultiApp
    app_type = TMAP8App
    input_files = tmap8.i
    execute_on = timestep_end

    # change this to the 'lib' directory of your TMAP8 installation
    library_path = '/path/to/tmap8/lib'
  []
[]

[Transfers]
  [tritium_source]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = tmap8
    source_variable = H3_production
    variable = tritium_source
    execute_on = timestep_end
  []
[]
