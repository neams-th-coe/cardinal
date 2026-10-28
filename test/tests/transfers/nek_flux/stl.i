[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = from_nek
      mesh = 'stl'
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6'
  #stl = "other_sphere.stl"
  stl = "../../nek_mesh/stl/small_sphere.stl"
  stl_translation = '0.5 0.5 0.5'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []

  [Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  []
[]

[Postprocessors]
  [read_flux]
    type = ElementIntegralVariablePostprocessor
    variable = flux
    block = 9999
  []
  [volume]
    type = VolumePostprocessor
    block = 9999
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

