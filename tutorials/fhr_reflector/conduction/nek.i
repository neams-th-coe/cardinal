fluid_solid_interface = '1 2 7'

[Mesh]
  type = NekRSMesh
  boundary = ${fluid_solid_interface}
  scaling = 0.006
[]

[Problem]
  type = NekRSProblem
  casename = 'fluid'

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = flux_integral
    []
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
  []

  [Dimensionalize]
    U = 0.0575
    T = 923.15
    dT = 10.0
    L = 0.006
    rho = 1962.13
    Cp = 2416.0
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [boundary_flux]
    type = NekHeatFluxIntegral
    boundary = ${fluid_solid_interface}
  []
  [max_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]
