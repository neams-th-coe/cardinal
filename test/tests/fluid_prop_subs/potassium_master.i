[Problem]
  kernel_coverage_check = false
  type = FEProblem
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [dummy]
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = PotassiumApp
    input_files = potassium.i
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
