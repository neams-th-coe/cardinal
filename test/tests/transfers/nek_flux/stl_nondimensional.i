[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = from_nek
      mesh = 'stl mirror'
    []
  []

  [Dimensionalize]
    L = 5.0
    dT = 50.0
    T = 25.0
    rho = 10.0
    Cp = 40.0
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
  [max_flux_stl]
    type = ElementExtremeValue
    variable = flux
    block = 9999
  []
  [min_flux_stl]
    type = ElementExtremeValue
    variable = flux
    block = 9999
    value_type = min
  []
  [max_flux_mirror]
    type = ElementExtremeValue
    variable = flux
    block = 0
  []
  [min_flux_mirror]
    type = ElementExtremeValue
    variable = flux
    block = 0
    value_type = min
  []

  [surface_area] # should be 4 * pi * (0.5*5)**2
    type = VolumePostprocessor
    block = 9999
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

