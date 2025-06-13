Re = 1460.0
dp = 0.06
cylinder_diameter = ${fparse 4.4 * dp}
density = 3.645
C = 3121.0
mu = 2.93e-5
power = 2400.0
inlet_area = ${fparse pi * cylinder_diameter^2 / 4.0}

[Mesh]
  type = NekRSMesh
  boundary = 4
  scaling = ${dp}
[]

[Problem]
  type = NekRSProblem
  casename = 'pb67'
  n_usrwrk_slots = 2

  [Dimensionalize]
    L = ${dp}
    U = ${fparse Re * mu / density / dp}
    T = 523.0
    dT = ${fparse power * dp / Re / inlet_area / mu / C}
    rho = ${density}
    Cp = ${C}
  []

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
    []
    [temperature]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_x]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_y]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_z]
      type = NekFieldVariable
      direction = from_nek
    []
  []

  synchronization_interval = parent_app
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  interval = 100
[]

[Postprocessors]
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
  [average_nek_pebble_T]
    type = NekSideAverage
    boundary = '4'
    field = temperature
  []
[]
