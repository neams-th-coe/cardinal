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
  [max_rel_err]
    type = TallyRelativeError
    value_type = max
  []
  [min_rel_err]
    type = TallyRelativeError
    value_type = min
  []
  [avg_rel_err]
    type = TallyRelativeError
    value_type = average
  []
  [power_1]
    type = PointValue
    variable = kappa_fission
    point = '0 0 0'
  []
  [power_2]
    type = PointValue
    variable = kappa_fission
    point = '0 0 4'
  []
  [power_3]
    type = PointValue
    variable = kappa_fission
    point = '0 0 8'
  []

  [std_dev_1]
    type = PointValue
    variable = kappa_fission_std_dev
    point = '0 0 0'
  []
  [std_dev_2]
    type = PointValue
    variable = kappa_fission_std_dev
    point = '0 0 4'
  []
  [std_dev_3]
    type = PointValue
    variable = kappa_fission_std_dev
    point = '0 0 8'
  []
[]

[Outputs]
  csv = true
[]
