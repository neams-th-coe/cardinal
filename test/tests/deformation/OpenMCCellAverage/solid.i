[Mesh]
  [cube]
     type = FileMeshGenerator
     file = '2blckt03.e'
  []
 []

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Variables]
  [temp]
    initial_condition = 500
  []
[]

[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = temp
  []
  [source]
    type = CoupledForce
    variable = temp
    v = power
  []
[]

[BCs]
  [surface]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 500.0
  []
[]

[Materials]
  [k_fuel]
    type = GenericConstantMaterial
    prop_values = '0.05'
    prop_names = 'thermal_conductivity'
    block = 'fuel'
  []
  [k_water]
    type = GenericConstantMaterial
    prop_values = '0.5'
    prop_names = 'thermal_conductivity'
    block = 'water'
  []
[]

[Postprocessors]
  [max_fuel_T]
    type = ElementExtremeValue
    variable = temp
    value_type = max
    block = 'fuel'
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = 'fuel'
    execute_on = transfer
  []
[]

[MultiApps]
  [smech]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'smech.i'
    execute_on = timestep_end
    # execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [./temp_to_smech]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = smech
    variable = temp
    source_variable = temp
    # execute_on = 'timestep_begin'
  []
  [dispx]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_x
    variable = disp_x
    from_multi_app = smech
    # execute_on = 'timestep_begin'
  []
  [dispy]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_y
    variable = disp_y
    from_multi_app = smech
    # execute_on = 'timestep_begin'
  []
  [dispz]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_z
    variable = disp_z
    from_multi_app = smech
    # execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
[]

[Outputs]
  exodus = true
[]
