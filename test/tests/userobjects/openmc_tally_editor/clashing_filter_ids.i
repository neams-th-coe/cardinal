!include openmc_base.i

[UserObjects]
  [filter_a]
    type = OpenMCDomainFilterEditor
    create_filter = true
    filter_id = 10
    filter_type = 'cell'
    bins = ''
  []
  [filter_b]
    type = OpenMCDomainFilterEditor
    filter_id = 10
    filter_type = 'cell'
    bins = ''
  []
[]