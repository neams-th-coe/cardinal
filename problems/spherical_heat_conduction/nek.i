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

  [./TimeStepper]
    type = NekTimeStepper
  [../]
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
