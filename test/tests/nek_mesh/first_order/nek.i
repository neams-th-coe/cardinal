[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  order = FIRST
  boundary = '1 2 3 4'
[]

# only here to avoid a re-gold
[Variables]
  [dummy]
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]

# The points provided to these postprocessors are the centroids of the elements that
# we wish to print the node coordinates for.
[Postprocessors]
  [num_elems]
    type = NekMeshInfoPostprocessor
    test_type = num_elems
  []
  [num_nodes]
    type = NekMeshInfoPostprocessor
    test_type = num_nodes
  []

  # coordinates of nodes of element 0
  [elem0_node0_x]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 0
    test_type = node_x
  []
  [elem0_node0_y]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 0
    test_type = node_y
  []
  [elem0_node0_z]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 0
    test_type = node_z
  []
  [elem0_node1_x]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 1
    test_type = node_x
  []
  [elem0_node1_y]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 1
    test_type = node_y
  []
  [elem0_node1_z]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 1
    test_type = node_z
  []
  [elem0_node2_x]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 2
    test_type = node_x
  []
  [elem0_node2_y]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 2
    test_type = node_y
  []
  [elem0_node2_z]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 2
    test_type = node_z
  []
  [elem0_node3_x]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 3
    test_type = node_x
  []
  [elem0_node3_y]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 3
    test_type = node_y
  []
  [elem0_node3_z]
    type = NekMeshInfoPostprocessor
    point = '0.180650573 0.0296807698 0.471347985'
    node = 3
    test_type = node_z
  []

  # coordinates of nodes of element 24
  [elem24_node0_x]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 0
    test_type = node_x
  []
  [elem24_node0_y]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 0
    test_type = node_y
  []
  [elem24_node0_z]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 0
    test_type = node_z
  []
  [elem24_node1_x]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 1
    test_type = node_x
  []
  [elem24_node1_y]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 1
    test_type = node_y
  []
  [elem24_node1_z]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 1
    test_type = node_z
  []
  [elem24_node2_x]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 2
    test_type = node_x
  []
  [elem24_node2_y]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 2
    test_type = node_y
  []
  [elem24_node2_z]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 2
    test_type = node_z
  []
  [elem24_node3_x]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 3
    test_type = node_x
  []
  [elem24_node3_y]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 3
    test_type = node_y
  []
  [elem24_node3_z]
    type = NekMeshInfoPostprocessor
    point = '0.230319 -0.0598418 0.365829'
    node = 3
    test_type = node_z
  []

  # coordinates of nodes of element 147
  [elem147_node0_x]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 0
    test_type = node_x
  []
  [elem147_node0_y]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 0
    test_type = node_y
  []
  [elem147_node0_z]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 0
    test_type = node_z
  []
  [elem147_node1_x]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 1
    test_type = node_x
  []
  [elem147_node1_y]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 1
    test_type = node_y
  []
  [elem147_node1_z]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 1
    test_type = node_z
  []
  [elem147_node2_x]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 2
    test_type = node_x
  []
  [elem147_node2_y]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 2
    test_type = node_y
  []
  [elem147_node2_z]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 2
    test_type = node_z
  []
  [elem147_node3_x]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 3
    test_type = node_x
  []
  [elem147_node3_y]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 3
    test_type = node_y
  []
  [elem147_node3_z]
    type = NekMeshInfoPostprocessor
    point = '-0.1404 0.314343 -0.456323'
    node = 3
    test_type = node_z
  []
[]
