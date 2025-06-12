# The mesh file used for nekRS (brick.re2) has six sidesets numbered
# as 1, 2, 3, 4, 5, 6. In the nekRS input file (brick.par), we set
# these six boundaries to have insulated, insulated, specified flux,
# insulated, insulated, and insulated boundary conditions, respectively.
# Based on the data transfers assumed in/out from nekRS, we should throw
# an error if a flux boundary condition is not specified on the boundary
# we set for NekRSMesh, because otherwise that heat flux condition would
# never get used.

[Mesh]
  type = NekRSMesh
  boundary = '2'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
