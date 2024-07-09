!include openmc_base.i

[UserObjects]
  [tally_scatter]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = 100
    scores = 'absorption'
    filter_ids = ''
    nuclides = 'U238'
  []
[]