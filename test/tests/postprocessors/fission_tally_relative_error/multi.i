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
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100'
    tally_score = 'heating kappa_fission'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100'
  cell_level = 0
  check_tally_sum = false

  # this outputs the fission tally standard deviation in space
  output = 'unrelaxed_tally_std_dev'

  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_rel_err_ht]
    type = TallyRelativeError
    value_type = max
    tally_score = 'heating'
  []
  [min_rel_err_ht]
    type = TallyRelativeError
    value_type = min
    tally_score = 'heating'
  []
  [avg_rel_err_ht]
    type = TallyRelativeError
    value_type = average
    tally_score = 'heating'
  []
  [max_rel_err_kf]
    type = TallyRelativeError
    value_type = max
    tally_score = 'kappa_fission'
  []
  [min_rel_err_kf]
    type = TallyRelativeError
    value_type = min
    tally_score = 'kappa_fission'
  []
  [avg_rel_err_kf]
    type = TallyRelativeError
    value_type = average
    tally_score = 'kappa_fission'
  []
[]

[Outputs]
  csv = true
[]
