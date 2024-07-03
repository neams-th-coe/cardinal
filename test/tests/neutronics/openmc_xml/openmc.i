[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[UserObjects]
  [material_filter]
    type = OpenMCDomainFilterEditor
    create_filter = true
    filter_id = 1
    filter_type = "material"
    bins = "1 2 3"
  []
  [tally_5]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = '5'
    scores = "scatter absorption"
    nuclides = "total U235"
    filter_ids = 1
  []
  [tally_1_nuclides]
    type = OpenMCTallyNuclides
    tally_id = 1
    names = "total U235"
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Debug]
  show_actions = true
[]

[Outputs]
  csv = true
[]