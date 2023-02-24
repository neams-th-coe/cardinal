[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[UserObjects]
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 100
  []
[]

[Outputs]
  exodus = true
[]
