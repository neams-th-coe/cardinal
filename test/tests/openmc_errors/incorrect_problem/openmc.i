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

[Postprocessors]
  [max_tally_rel_err]
    type = FissionTallyRelativeError
    value_type = max
  []
[]

[Outputs]
  exodus = true
[]
