Re = 4.8e4                              # (-)
mu = 0.011266321                        # Pa-s
rho = 4147.3                            # kg/m3

[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
[]

[Problem]
  type = NekRSProblem
  casename = 'msfr'
  output = 'temperature'

  synchronization_interval = parent_app

  [Dimensionalize]
    L = 1.0
    U = ${fparse Re * mu / rho}
    T = ${fparse 625.0 + 273.15}
    dT = 100.0
    rho = ${rho}
    Cp = 1524.86 # J/kg/K
  []

  normalization_abs_tol = 1e6
  normalization_rel_tol = 1e-3
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
    min_dt = 1e-10
  []
[]

[Outputs]
  exodus = true
  hide = 'source_integral'
[]
