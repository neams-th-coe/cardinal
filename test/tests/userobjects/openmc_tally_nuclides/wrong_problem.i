[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 8
    ny = 8
    nz = 8
    xmin = -12.5
    xmax = 87.5
    ymin = -12.5
    ymax = 37.5
    zmin = -12.5
    zmax = 12.5
  []
[]

[Variables]
  [u]
    order = FIRST
  []
[]

[Kernels]
  [d]
    type = Diffusion
    variable = u
  []
[]

[Problem]
  type = FEProblem
[]

[UserObjects]
  [tally1]
    type = OpenMCTallyNuclides
    tally_id = 2
    names = 'U238'
  []
[]

[Postprocessors]
  [power_1]
    type = PointValue
    variable = kappa_fission
    point = '0.0 -12.0 0.0'
  []
  [power_2]
    type = PointValue
    variable = kappa_fission
    point = '0.0 37.0 0.0'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
