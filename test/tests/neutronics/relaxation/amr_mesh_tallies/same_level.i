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

  [Tallies/Mesh]
    normalize_by_global_tally := false
  []
[]

[Outputs]
  hide := 'uniform'
[]
