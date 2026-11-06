!include tmap8.i

[MultiApps]
  [openmc]
    type = FullSolveMultiApp
    app_type = CardinalApp
    input_files = openmc.i
    execute_on = initial

    # change this to the 'lib' directory of your Cardinal repository
    library_path = '/path/to/cardinal/lib'
  []
[]

[Transfers]
  [tritium_source]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = openmc
    source_variable = H3_production
    variable = tritium_source
    execute_on = initial
  []
[]
