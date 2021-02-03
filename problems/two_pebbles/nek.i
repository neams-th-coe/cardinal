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
  [nek_exo]
    type = Exodus
    output_dimension = 3
    interval = 100
  []
[]

[Postprocessors]
  [total_flux]
    type = Receiver
  []
[]
