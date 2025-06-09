# copy-pasta from common_input.i

channel_diameter = 0.016                 # diameter of the coolant channels (m)
height = 6.343                           # height of the full core (m)
inlet_T = 598.0                          # inlet fluid temperature (K)
power = 200e6                            # full core power (W)
mdot = 117.3                             # fluid mass flowrate (kg/s)
fluid_density = 5.5508                   # fluid density (kg/m3)
fluid_Cp = 5189.0                        # fluid isobaric specific heat (J/kg/K)
n_bundles = 12                           # number of bundles in the full core
n_coolant_channels_per_block = 108       # number of coolant channels per assembly
unit_cell_height = 1.6                   # unit cell height - arbitrarily selected

[Mesh]
  type = NekRSMesh
  boundary = '3'
  volume = true
  scaling = ${channel_diameter}
[]

[Problem]
  type = NekRSProblem
  casename = 'ranstube'
  n_usrwrk_slots = 2

  [Dimensionalize]
    U = ${fparse mdot / (n_bundles * n_coolant_channels_per_block) / fluid_density / (pi * channel_diameter * channel_diameter / 4.0)}
    T = ${inlet_T}
    dT = ${fparse power / mdot / fluid_Cp * unit_cell_height / height}
    L = ${channel_diameter}
    rho = ${fluid_density}
    Cp = ${fluid_Cp}
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

  [screen]
    type = Console
    hide = 'flux_integral transfer_in'
  []

  [csv]
    file_base = 'csv/nek'
    type = CSV
  []
[]

[Postprocessors]
  [inlet_T]
    type = NekSideAverage
    field = temperature
    boundary = '1'
  []
  [outlet_T]
    type = NekSideAverage
    field = temperature
    boundary = '2'
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
[]
