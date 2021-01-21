[Problem]
  # We dont need to use NekProblem here for this simple test, which only checks
  # the construction of a mesh. Therefore, we are implicitly using FEProblem, which
  # has some requirements for the existence of kernels and variables. Disable those
  # with 'kernel_coverage_check = false' and adding a 'dummy' nonlinear variable.
  kernel_coverage_check = false
[]

[Mesh]
  type = NekRSMesh
  order = FIRST
  boundary = '1 2 3 4'
[]

[Variables]
  [dummy]
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [./num_elems]
    type = NekMeshInfoPostprocessor
    test_type = num_elems
  [../]
  [./num_nodes]
    type = NekMeshInfoPostprocessor
    test_type = num_nodes
  [../]

  # coordinates of nodes of element 0
  [./elem0_node0_x]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 0
    test_type = node_x
  [../]
  [./elem0_node0_y]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 0
    test_type = node_y
  [../]
  [./elem0_node0_z]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 0
    test_type = node_z
  [../]
  [./elem0_node1_x]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 1
    test_type = node_x
  [../]
  [./elem0_node1_y]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 1
    test_type = node_y
  [../]
  [./elem0_node1_z]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 1
    test_type = node_z
  [../]
  [./elem0_node2_x]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 2
    test_type = node_x
  [../]
  [./elem0_node2_y]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 2
    test_type = node_y
  [../]
  [./elem0_node2_z]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 2
    test_type = node_z
  [../]
  [./elem0_node3_x]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 3
    test_type = node_x
  [../]
  [./elem0_node3_y]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 3
    test_type = node_y
  [../]
  [./elem0_node3_z]
    type = NekMeshInfoPostprocessor
    element = 0
    node = 3
    test_type = node_z
  [../]

  # coordinates of nodes of element 24
  [./elem24_node0_x]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 0
    test_type = node_x
  [../]
  [./elem24_node0_y]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 0
    test_type = node_y
  [../]
  [./elem24_node0_z]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 0
    test_type = node_z
  [../]
  [./elem24_node1_x]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 1
    test_type = node_x
  [../]
  [./elem24_node1_y]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 1
    test_type = node_y
  [../]
  [./elem24_node1_z]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 1
    test_type = node_z
  [../]
  [./elem24_node2_x]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 2
    test_type = node_x
  [../]
  [./elem24_node2_y]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 2
    test_type = node_y
  [../]
  [./elem24_node2_z]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 2
    test_type = node_z
  [../]
  [./elem24_node3_x]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 3
    test_type = node_x
  [../]
  [./elem24_node3_y]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 3
    test_type = node_y
  [../]
  [./elem24_node3_z]
    type = NekMeshInfoPostprocessor
    element = 24
    node = 3
    test_type = node_z
  [../]

  # coordinates of nodes of element 147
  [./elem147_node0_x]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 0
    test_type = node_x
  [../]
  [./elem147_node0_y]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 0
    test_type = node_y
  [../]
  [./elem147_node0_z]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 0
    test_type = node_z
  [../]
  [./elem147_node1_x]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 1
    test_type = node_x
  [../]
  [./elem147_node1_y]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 1
    test_type = node_y
  [../]
  [./elem147_node1_z]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 1
    test_type = node_z
  [../]
  [./elem147_node2_x]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 2
    test_type = node_x
  [../]
  [./elem147_node2_y]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 2
    test_type = node_y
  [../]
  [./elem147_node2_z]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 2
    test_type = node_z
  [../]
  [./elem147_node3_x]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 3
    test_type = node_x
  [../]
  [./elem147_node3_y]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 3
    test_type = node_y
  [../]
  [./elem147_node3_z]
    type = NekMeshInfoPostprocessor
    element = 147
    node = 3
    test_type = node_z
  [../]
[]
