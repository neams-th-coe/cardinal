Re = 1460.0
dp = 0.06
cylinder_diameter = ${fparse 4.4 * dp}
rho = 3.645
Cp = 3121.0
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
  output = 'velocity'
  has_heat_source = false
  n_usrwrk_slots = 2

  nondimensional = true
  L_ref = ${dp}
  U_ref = ${fparse Re * mu / rho / dp}
  T_ref = 523.0
  dT_ref = ${fparse power * dp / Re / inlet_area / mu / Cp}

  rho_0 = ${rho}
  Cp_0 = ${Cp}

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
