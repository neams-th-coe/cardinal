[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[Adaptivity]
  [Indicators/optical_depth_hmin]
    type = ElementOpticalDepthIndicator
    rxn_rate = 'total'
    h_type = 'min'
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Steady
[]
