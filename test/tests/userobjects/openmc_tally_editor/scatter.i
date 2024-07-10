!include openmc_base.i

[UserObjects]
  [tally_scatter]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = 100
    scores = 'scatter'
    filter_ids = ''
    nuclides = ''
  []
[]
