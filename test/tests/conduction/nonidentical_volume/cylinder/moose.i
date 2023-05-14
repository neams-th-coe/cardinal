[Mesh]
  [circle]
    type = AnnularMeshGenerator
    rmin = 0
    rmax = 0.75
    nt = 60
    nr = 20
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = circle
    heights = '0.5'
    num_layers = '10'
    direction = '0 0 1'
  []
  [translate]
    type = TransformGenerator
    input = extrude
    transform = translate
    vector_value = '0 0 -0.25'
  []
[]

[Variables]
  [temperature]
    initial_condition = 500.0
  []
[]

[AuxVariables]
  [source]
  []
  [axial]
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
  []
  [source]
    type = CoupledForce
    variable = temperature
    v = source
  []
[]

a=0.5
o=0.15

[Functions]
  [axial]
    type = ParsedFunction
    expression = '10000*(z-0.25)*(z-0.25)*(8*${a}*${a}*${a}/((x-${o})*(x-${o})+4*${a}*${a}))*(8*${a}*${a}*${a}/((y-${o})*(y-${o})+4*${a}*${a}))'
  []
[]

[AuxKernels]
  [axial]
    type = FunctionAux
    variable = axial
    function = axial
  []
  [source]
    type = ParsedAux
    variable = source
    function = 'axial+temperature'
    coupled_variables = 'axial temperature'
    execute_on = 'linear'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = '1'
    value = 500.0
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [source]
    type = ElementIntegralVariablePostprocessor
    variable = source
  []
  [max_T]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
  []
  [min_T]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
  []
  [avg_T_volume]
    type = ElementAverageValue
    variable = temperature
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

