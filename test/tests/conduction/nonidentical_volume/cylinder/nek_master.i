# The master app performs a dummy solve - we are just testing the ability for nekRS
# to send temperature to BISON, which performs some operation on it to return a heat
# source to nekRS. Here, that operation is to multiply temperature by 1 plus some
# spatial variation in the axial and radial directions.

[Mesh]
  [circle]
    type = AnnularMeshGenerator
    rmin = 0
    rmax = 0.75
    nt = 90
    nr = 15
    growth_r = -1.2
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
  [dummy]
  []
[]

[AuxVariables]
  [axial]
  []
  [source]
  []
  [nek_temp]
    # This initial value will be used in the first heat source sent to nekRS
    # because MOOSE runs first
    initial_condition = 500.0
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
    expression = 'axial+nek_temp'
    coupled_variables = 'axial nek_temp'
    execute_on = 'linear'
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = dummy
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = dummy
    value = 1.0
    boundary = '1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 15
  dt = 0.1
  nl_abs_tol = 1e-8
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [source]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = source
    to_multi_app = nek
    variable = heat_source
  []
  [source_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = source_integral
    from_postprocessor = source_integral
    to_multi_app = nek
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
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = source
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
  hide = 'axial'
[]
