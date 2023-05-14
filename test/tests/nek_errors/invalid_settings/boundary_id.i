# The mesh file used for nekRS (brick.re2) has six sidesets numbered
# as 1, 2, 3, 4, 5, 6. Trying to construct the surface mesh corresponding
# to boundary 8 (a non-existent boundary ID) should throw an error.

[Mesh]
  type = NekRSMesh
  boundary = 8
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
