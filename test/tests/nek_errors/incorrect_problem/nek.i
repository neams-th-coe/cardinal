[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = dummy
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [nek_avg_temp]
    type = NekVolumeAverage
    field = temperature
  []
[]
