!include openmc_base.i

[UserObjects]
  [new_filter]
    type = OpenMCDomainFilterEditor
    create_filter = true
    filter_id = 100
    filter_type = 'cell'
    bins = '1'
  []
  [tally_scatter]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = 100
    scores = 'absorption'
    filter_ids = '100'
    nuclides = ''
  []
[]