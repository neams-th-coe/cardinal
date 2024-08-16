[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = FEProblem

  [Filters]
    [Energy]
      type = EnergyFilter
      # CASMO 2 group structure for testing. May result in some missed particles
      energy_boundaries = '0.0 6.25e-1 2.0e7'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
