[Mesh]
  type = NekRSMesh
  volume = true
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
