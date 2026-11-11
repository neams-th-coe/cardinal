[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      conserve_flux_by_sideset = true
      mesh = stl
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
  stl = '../../nek_mesh/stl/small_sphere.stl'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
