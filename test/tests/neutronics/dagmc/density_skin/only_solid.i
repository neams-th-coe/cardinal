[Mesh]
  type = FileMesh
  file = ../mesh_tallies/slab.e
  allow_renumbering = false
[]

[AuxVariables]
  [density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  initial_properties = xml
  temperature_blocks = '1 2'
  cell_level = 0
  power = 100.0

  skinner = moab

  [Tallies]
    [Mesh]
      type = CellTally
      block = '1 2'
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp density'
    fields_min = '0 0'
    fields_max = '1000 100'
    n_field_bins = '1 4'

    # just one temperature bin


    build_graveyard = true
  []
[]

[Executioner]
  type = Steady
[]
