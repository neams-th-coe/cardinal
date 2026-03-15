joule_per_ev = 1.60218e-19

q      = ${fparse 1.0e6}                 # energy release per absorption (eV)
Sigma0 = ${fparse 4.0 * 0.025}           # Initial macroscopic XS, 1/cm
T0     = 293.6                           # surface temperature (K)
Y0     = 6.65e11                         # source intensity (n / cm2-s)

!include mesh.i

[AuxVariables]
  [temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []

  # Variables added to convert into proper heat source term for the power in the solid
  [heat_source] # W / cm^3
    family = MONOMIAL
    order = CONSTANT
  []
  [dummy_zero]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []

  # Change the unit of the 'flux' (neutrons / m^2 / s) into a volumetric power. This is
  # based on Eq. (5b) in the paper, which shows the volumetric power is q * Sigma * flux
  [changing_units]
    type = ParsedAux
    variable = heat_source
    coupled_variables = 'flux temp'
    expression = 'flux * ${q} * ${joule_per_ev} * ${Sigma0} * sqrt(${T0} / temp)'
    execute_on = 'timestep_end'
  []

  # Compute the "density" with inverse square root Doppler boadening
  [compute_density]
    type = ParsedAux
    variable = density
    coupled_variables = 'temp'
    expression = 'sqrt(${T0} / temp)'
    execute_on = 'timestep_end'
  []
[]


[ICs]
  [density]
    type = ConstantIC
    variable = density
    value = 1.0
  []
  [temp]
    type = ConstantIC
    variable = temp
    value = ${T0}
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = false
  source_strength = ${fparse Y0 * 2.0 * 2.0} # multiply by the left area to get units of neutrons / s

  cell_level = 0
  density_blocks = '0'
  mgxs_reference_densities = '1.0'
  initial_properties = xml

  particles = 1000
  batches = 100
  inactive_batches = 0

  inactive_distance = 0.0
  active_distance = 209.0

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
    error_on_miss = false
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
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
