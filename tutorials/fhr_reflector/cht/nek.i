fluid_solid_interface = '1 2 7'

[Mesh]
  type = NekRSMesh
  boundary = ${fluid_solid_interface}
  scaling = 0.006
[]

[Problem]
  type = NekRSProblem

  nondimensional = true
  U_ref = 0.0575
  T_ref = 923.15
  dT_ref = 10.0
  L_ref = 0.006
  rho_0 = 1962.13
  Cp_0 = 2416.0
[]

[Executioner]
  type = Transient
  timestep_tolerance = 1e-9

  [./TimeStepper]
    type = NekTimeStepper
  [../]
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
  [pressure_in]
    type = NekSideAverage
    field = pressure
    boundary = '5'
  []
  [mdot_in]
    type = NekMassFluxWeightedSideIntegral
    field = unity
    boundary = '5'
  []
  [mdot_out]
    type = NekMassFluxWeightedSideIntegral
    field = unity
    boundary = '6'
  []
[]
