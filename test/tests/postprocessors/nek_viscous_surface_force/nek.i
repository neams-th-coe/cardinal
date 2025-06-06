[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Functions]
  [side1x]
    type = ParsedFunction
    expression = '20*(1+2)'
  []
  [side1y]
    type = ParsedFunction
    expression = '20*(2+2)'
  []
  [side1z]
    type = ParsedFunction
    expression = '20*(3+2)'
  []
[]

[AuxVariables]
  [f1]
  []
  [f2]
  []
  [f3]
  []
[]

[AuxKernels]
  [f1]
    type = FunctionAux
    variable = f1
    function = side1x
  []
  [f2]
    type = FunctionAux
    variable = f2
    function = side1y
  []
  [f3]
    type = FunctionAux
    variable = f3
    function = side1z
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  hide = 'f1 f2 f3'
[]

[Postprocessors]
  [viscous_1]
    type = NekViscousSurfaceForce
    boundary = '1'
    mesh = 'fluid'
  []
  [compare_x]
    type = SideIntegralVariablePostprocessor
    variable = f1
    boundary = '1'
  []
  [compare_y]
    type = SideIntegralVariablePostprocessor
    variable = f2
    boundary = '1'
  []
  [compare_z]
    type = SideIntegralVariablePostprocessor
    variable = f3
    boundary = '1'
  []
  [sum]
    type = ParsedPostprocessor
    expression = 'sqrt(compare_x*compare_x+compare_y*compare_y+compare_z*compare_z)'
    pp_names = 'compare_x compare_y compare_z'
  []
[]
