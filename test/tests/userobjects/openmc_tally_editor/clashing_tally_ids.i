!include openmc_base.i

[UserObjects]
  [tally_a]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = 10
    scores = 'absorption'
    filter_ids = ''
    nuclides = 'U238'
  []
  [tally_b]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = 10
    scores = 'absorption'
    filter_ids = ''
    nuclides = 'U238'
  []
[]