[Mesh]
  type = NekMesh
[]

[Variables]
  [dummy]
  []
[]

[AuxVariables]
  [avg_flux]
  []
[]

[Problem]
  type = NekProblem
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  [exo]
    type = Exodus
    output_dimension = 3
  []
[]

[Postprocessors]
  [total_flux]
    type = Receiver
    default = 0
  []
[]
