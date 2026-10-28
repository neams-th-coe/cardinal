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
  stl = "small_sphere.stl"
  stl_translation = '1.0 1.0 1.0'
  stl_multiplier = 0.5
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
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

