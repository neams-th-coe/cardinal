!include ../mesh_tallies/openmc.i

[Adaptivity]
  marker = uniform

  [Markers/uniform]
    type = UniformMarker
    mark = 'DO_NOTHING'
  []
[]

[Problem]
  relaxation := constant
  relaxation_factor := 0.5
  output_cell_mapping := false

  xml_directory = '../mesh_tallies'

  # Keep the statepoint next to this input. Left at its default, it goes next to the
  # included input, where another test spec may be writing the same file at the same time.
  statepoint_prefix = './'

  [Tallies/Mesh]
    normalize_by_global_tally := false
  []
[]

[Outputs]
  hide := 'uniform'
[]
