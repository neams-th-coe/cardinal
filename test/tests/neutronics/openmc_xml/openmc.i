[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[TallyGeneratorAction]
    ids = '5 6 7'
    nuclides = true
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Debug]
  show_actions = true
[]

[Outputs]
  csv = true
[]

[Controls]
 [webserver]
   type = WebServerControl
   execute_on = "TIMESTEP_BEGIN TIMESTEP_END"
   port = 8000
 []
[]
