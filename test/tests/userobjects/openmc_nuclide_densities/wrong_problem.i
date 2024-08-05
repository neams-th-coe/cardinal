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

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[Problem]
  type = FEProblem
[]

[UserObjects]
  [mat1]
    type = OpenMCNuclideDensities
    material_id = 1
    names = 'U235 U238'
    densities = '0.01 0.02'
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
