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
    []
    [add_eeid]
        type = ParsedElementIDMeshGenerator
        input = generated_mesh
        extra_element_integer_names = ''
    []
[]
