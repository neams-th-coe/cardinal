joule_per_ev = 1.60218e-19

q      = ${fparse 1.0e6}       # Energy release per absorption (eV)
Sigma0 = ${fparse 4.0 * 0.025} # Initial macroscopic XS, 1/cm
T0     = 293.6                 # Surface temperature (K)
Y0     = 6.65e11               # Source intensity (n / cm2-s)
alpha  = -0.0001               # Linear doppler Coefficient (1 / K)
k      = 0.006                 # Thermal conductivity (W / cm-K)

!include mesh.i

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []

  # Variables added to convert into proper heat source term for the power in the solid
  [heat_source] # W / cm^3
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []

  # Change the unit of the 'flux' (neutrons / m^2 / s) into a volumetric power. This is
  # based on Eq. (5b) in the paper, which shows the volumetric power is q / k * Sigma * flux
  [compute_power]
    type = ParsedAux
    variable = heat_source
    coupled_variables = 'flux temp'
    expression = 'flux * ${q} * ${joule_per_ev} * ${Sigma0} * (1+(${alpha}/${Sigma0})*(temp-${T0}))'
    execute_on = 'timestep_end'
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = ${T0}
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  source_strength = ${fparse Y0 * 2.0 * 2.0} # multiply by the left area to get units of neutrons / s

  cell_level = 0
  temperature_blocks = '0'
  initial_properties = xml

  particles = 1000
  batches = 100
  inactive_batches = 0

  relaxation = 'constant'

  [Tallies]
    [heating]
      type = CellTally
      score = 'flux'
      output = 'unrelaxed_tally_std_dev'
      check_tally_sum = false
    []
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  steady_state_detection = true
  check_aux = true
  num_steps = 3
[]

[MultiApps]
  [solid]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solid.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [solid_temp]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = T
    variable = temp
    from_multi_app = solid
  []
  [source_to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = heat_source
    variable = power
    to_multi_app = solid
  []
[]

[Postprocessors]
  [max_heat_source]
    type = ElementExtremeValue
    variable = heat_source
    execute_on = 'timestep_begin'
  []
  [max_flux]
    type = ElementExtremeValue
    variable = flux
  []
  [max_temp]
    type = ElementExtremeValue
    variable = temp
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
