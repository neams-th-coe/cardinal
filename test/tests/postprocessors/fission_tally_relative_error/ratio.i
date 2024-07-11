[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
    tally_score = 'heating kappa_fission'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0
  check_tally_sum = false

  output = 'unrelaxed_tally_std_dev'
  initial_properties = xml
[]

# the ratio of the std_dev variable and the output tally should give
# the relative error in the tallies.out
[AuxVariables]
  [heating_rel_err]
    family = MONOMIAL
    order = CONSTANT
  []
  [kappa_fission_rel_err]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [heating]
    type = ParsedAux
    variable = heating_rel_err
    expression = 'heating_std_dev / heating'
    coupled_variables = 'heating_std_dev heating'
  []
  [kappa_fission]
    type = ParsedAux
    variable = kappa_fission_rel_err
    expression = 'kappa_fission_std_dev / kappa_fission'
    coupled_variables = 'kappa_fission_std_dev kappa_fission'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heating_max]
    type = ElementExtremeValue
    value_type = max
    variable = heating_rel_err
  []
  [heating_min]
    type = ElementExtremeValue
    value_type = min
    variable = heating_rel_err
  []
  [kf_max]
    type = ElementExtremeValue
    value_type = max
    variable = kappa_fission_rel_err
  []
  [kf_min]
    type = ElementExtremeValue
    value_type = min
    variable = kappa_fission_rel_err
  []
  [max_ht]
    type = TallyRelativeError
    value_type = max
    tally_score = 'heating'
  []
  [min_ht]
    type = TallyRelativeError
    value_type = min
    tally_score = 'heating'
  []
  [avg_ht]
    type = TallyRelativeError
    value_type = average
    tally_score = 'heating'
  []
  [max_kf]
    type = TallyRelativeError
    value_type = max
    tally_score = 'kappa_fission'
  []
  [min_kf]
    type = TallyRelativeError
    value_type = min
    tally_score = 'kappa_fission'
  []
  [avg_kf]
    type = TallyRelativeError
    value_type = average
    tally_score = 'kappa_fission'
  []
[]

[Outputs]
  csv = true
[]
