inlet_T  = 573.0           # inlet temperature
power = 1e3                # total power (W)
Re = 500.0                 # Reynolds number

pin_diameter = 0.97e-2     # pin outer diameter
pin_pitch = 1.28e-2        # pin pitch

mu = 8.8e-5                # fluid dynamic viscosity
rho = 723.6                # fluid density
Cp = 5512.0                # fluid isobaric specific heat capacity

flow_area = ${fparse pin_pitch * pin_pitch - pi * pin_diameter * pin_diameter / 4.0}
wetted_perimeter = ${fparse pi * pin_diameter}
hydraulic_diameter = ${fparse 4.0 * flow_area / wetted_perimeter}

U_ref = ${fparse Re * mu / rho / hydraulic_diameter}
mdot = ${fparse rho * U_ref * flow_area}
dT = ${fparse power / mdot / Cp}

[Problem]
  type = NekRSProblem
  casename = 'fluid'

  nondimensional = true
  L_ref = ${hydraulic_diameter}
  T_ref = ${inlet_T}
  U_ref = ${U_ref}
  dT_ref = ${dT}

  rho_0 = ${rho}
  Cp_0 = ${Cp}

  has_heat_source = false

  # this just reduces how often we send data back up to the solid app
  synchronization_interval = parent_app
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
  scaling = ${hydraulic_diameter}
  volume = true
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
