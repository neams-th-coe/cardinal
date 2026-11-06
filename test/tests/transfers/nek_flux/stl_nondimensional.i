[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = from_nek
      mesh = 'stl mirror'
      scalar = "temperature"
    []
    [mass_flux]
      type = NekBoundaryFlux
      direction = from_nek
      mesh = 'stl mirror'
      scalar = "two"
    []
  []

  [Dimensionalize]
    L = 5.0
    dT = 50.0
    T = 25.0
    rho = 10.0
    Cp = 40.0

    ds02 = 7500.0
    s02 = 1000.0
    transport_coeff_2 = 6.5e2
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6'
  stl = "../../nek_mesh/stl/small_sphere.stl"
  stl_translation = '0.5 0.5 0.5'
  scaling = 5.0
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [surface_area] # should be 4 * pi * (0.5*5)**2
    type = VolumePostprocessor
    block = 9999
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

