[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    extra_element_integers = 'extreme_value'
  []
  [add_eeid]
    type = ParsedElementIDMeshGenerator
    input = generated_mesh
    extra_element_integer_names = 'id_1 id_2 id_3' #names of the eeid
    values = '-1 2 3 4 5'
  []
[]
