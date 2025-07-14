[Mesh]
  type = FileMesh
  file = ../meshes/pyramid.exo
  uniform_refine = 2
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [temp_test]
    order = SECOND
  []
[]

[ICs]
  [temp_test]
    type = FunctionIC
    variable = temp_test
    function = temp
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = '25'
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = '100*(exp(x)+exp(y)+exp(z)+sin(y)+x*y*z)+500*y+500*x'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 1
  l_max_its = 25
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [flux_side1]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '1'
  []
  [flux_side2]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '2'
  []
  [flux_side3]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '3'
  []
  [flux_side4]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '4'
  []
  [flux_side5]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '5'
  []
  [flux_side6]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '6'
  []
  [flux_side7]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '7'
  []
  [flux_side8]
    type = SideDiffusiveFluxIntegral
    variable = temp_test
    diffusivity = k
    boundary = '8'
  []
[]
