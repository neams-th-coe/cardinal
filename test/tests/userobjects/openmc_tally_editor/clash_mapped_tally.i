!include openmc_base.i

[UserObjects]
  [tally_a]
    type = OpenMCTallyEditor
    tally_id = 3
    scores = 'absorption'
    filter_ids = ''
    nuclides = 'U238'
  []
[]
