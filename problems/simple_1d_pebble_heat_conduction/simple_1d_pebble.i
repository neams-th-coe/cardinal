[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = BodyForce
    value = 1e9
    variable = temp
  []
[]

[Mesh]
  type = GeneratedMesh
  nx = 10
  dim = 1
  xmax = 0.015
[]

[BCs]
  [outside]
    type = DirichletBC
    variable = temp
    boundary = 'right'
    value = 300
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '150' # W/mK
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Steady
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'
[]

[Variables]
  [temp]
  []
[]

[Outputs]
  exodus = true
[]
