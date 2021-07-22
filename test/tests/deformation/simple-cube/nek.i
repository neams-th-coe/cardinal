[Mesh]
  type = NekRSMesh
  order = FIRST
  volume = true
[]

[Problem]
  type = NekRSProblem
  moving_mesh = true
[]

[Functions]
  [temp_ansol]
    type = ParsedFunction
    value = (sin(x)*sin(y)*sin(z))+5
  []
[]

[Executioner]
  type = Transient
  [./TimeStepper]
    type = NekTimeStepper
  [../]
[]

[Postprocessors]
  [./integral]
    type = ElementL2Error
    variable = temp
    function = temp_ansol
  [../]
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  hide = 'heat_source source_integral transfer_in'
[]

