[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Problem]
  type = FEProblem

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = '1'
    []
  []
[]

[Executioner]
  type = Transient
[]
