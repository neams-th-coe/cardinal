[Mesh]
  type = FileMesh
  file = mesh2_in.e
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [u_interpolation]
  []
  [u_mesh_function]
  []
  [u_nearest_node]
  []
  [u_projection]
  []
  [u_sample]
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
