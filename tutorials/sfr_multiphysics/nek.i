Re = 1068
n_pitches = 3

n_pins_in_bundle = 7
L_ref = 7.646e-3                                 # characteristic length
mu_0 = 2.37e-4                                   # reference viscosity
L = 20.32e-2                                     # axial height of one wire pitch
inlet_area = 2.568025e-4                         # inlet flow area
power = 1000e6                                   # total core power
n_bundles = 180                                  # number of fueled (driver) assemblies
n_pins = 271                                     # number of pins per bundle
active_height = 0.8582                           # active height of fuel pins
n_axial_pitches = ${fparse active_height / L}    # number of wire pitches per height
power_per_pitch = ${fparse power / (n_bundles * n_pins) / n_axial_pitches * n_pins_in_bundle}

# approximate total power per pin (corrected based on reynolds number and the nominal dT)
nominal_dT_per_axial_pitch = ${fparse 155.0 / n_axial_pitches}
nominal_dT = ${fparse nominal_dT_per_axial_pitch * n_pitches}
nominal_mdot = ${fparse power_per_pitch * n_pitches / 1228.0 / nominal_dT}
nominal_Re = ${fparse nominal_mdot * L_ref / inlet_area / mu_0}

[Mesh]
  type = NekRSMesh
  boundary = '1'
  volume = true
  scaling = ${L_ref}
  fixed_meshes = true
[]

[Problem]
  type = NekRSProblem
  has_heat_source = false

  nondimensional = true
  L_ref = ${L_ref}
  T_ref = 628.15
  dT_ref = ${nominal_dT}
  U_ref = ${fparse Re * mu_0 / 834.5 / L_ref}
  rho_0 = 834.5
  Cp_0 = 1228.0
[]

[AuxVariables]
  [axial_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [axial_temp]
    type = SpatialUserObjectAux
    variable = axial_temp
    execute_on = timestep_end
    user_object = layered_average
  []
[]

[UserObjects]
  [layered_average]
    type = LayeredAverage
    direction = z
    num_layers = ${fparse 80 * n_pitches}
    variable = temp
  []
[]

[Executioner]
  type = Transient
  timestep_tolerance = 1e-3

  [./TimeStepper]
    type = NekTimeStepper
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'flux_integral outlet_T'
[]

[Postprocessors]
  [outlet_T]
    type = NekMassFluxWeightedSideAverage
    field = temperature
    boundary = '3'
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [dT]
    type = LinearCombinationPostprocessor
    pp_names = 'outlet_T'
    pp_coefs = '1.0'
    b = '-628.15'
  []
  [heat_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
[]
