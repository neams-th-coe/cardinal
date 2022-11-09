[Mesh]
  [clad] # This makes a circular annulus that will represent the clad
    type = AnnularMeshGenerator
    nr = 1
    nt = 6
    rmin = 0.0
    rmax = 1.0
  []
  [to_hex20]
    type = NekMeshGenerator
    input = clad
  []

  parallel_type = replicated
[]
