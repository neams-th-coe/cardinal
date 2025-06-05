[Mesh]
  type = FileMesh
  file = nek_out.e
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [load_nek_solution]
    type = SolutionUserObject
    mesh = nek_out.e
    system_variables = 'temperature'
  []
[]

[AuxVariables]
  [nek_temp]
  []
[]

[AuxKernels]
  [nek_temp]
    type = SolutionAux
    solution = load_nek_solution
    variable = nek_temp
    from_variable = temperature
  []
[]

[Executioner]
  type = Transient
  dt = 2e-3
  num_steps = 10
[]

[Outputs]
  exodus = true
[]
