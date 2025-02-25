!include common_input.i

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
num_layers_for_plots = 50                # number of layers to average fields over for plotting

[Mesh]
  type = NekRSMesh
  boundary = '3'
  volume = true
  scaling = ${channel_diameter}
[]

[Problem]
  type = NekRSProblem
  casename = 'ranstube'
  has_heat_source = false

  [Dimensionalize]
    U = ${fparse mdot / (n_bundles * n_coolant_channels_per_block) / fluid_density / (pi * channel_diameter * channel_diameter / 4.0)}
    T = ${inlet_T}
    dT = ${fparse power / mdot / fluid_Cp * unit_cell_height / height}
    L = ${channel_diameter}
    rho = ${fluid_density}
    Cp = ${fluid_Cp}
  []

  synchronization_interval = parent_app
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

  [screen]
    type = Console
    hide = 'boundary_flux inlet_T outlet_T max_T flux_integral transfer_in'
  []

  [csv]
    file_base = 'csv/nek'
    type = CSV
  []
[]

[Postprocessors]
  [boundary_flux]
    type = NekHeatFluxIntegral
    boundary = '3'
  []
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

[UserObjects]
active = '' # simply comment out this line to evaluate these user objects
  [layered_bin]
    type = LayeredBin
    num_layers = ${num_layers_for_plots}
    direction = z
  []
  [wall_temp]
    type = NekBinnedSideAverage
    bins = 'layered_bin'
    boundary = '3'
    field = temperature
    map_space_by_qp = true
  []
  [bulk_temp]
    type = NekBinnedVolumeAverage
    bins = 'layered_bin'
    field = temperature
    map_space_by_qp = true
  []
[]

[VectorPostprocessors]
active = '' # simply comment out this line to evaluate these user objects
  [wall]
    type = SpatialUserObjectVectorPostprocessor
    userobject = wall_temp
  []
  [bulk]
    type = SpatialUserObjectVectorPostprocessor
    userobject = bulk_temp
  []
[]
