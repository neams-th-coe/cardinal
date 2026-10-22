[Mesh]
  [cube_1]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1'
    dy = '1'
    dz = '1'
    ix = '1'
    iy = '1'
    iz = '1'
    subdomain_id = '0'
  []
  [cube_2]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1'
    dy = '1'
    dz = '1'
    ix = '1'
    iy = '1'
    iz = '1'
    subdomain_id = '0'
  []
  [to_tet]
    type = ElementsToTetrahedronsConverter
    input = cube_2
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'cube_1 to_tet'
    positions = '0.0 0.0 0.0 1.0 0.0 0.0'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0

  [Tallies]
    [Mesh]
      type = XDGMeshTally
    []
  []
[]

[Executioner]
  type = Steady
[]
