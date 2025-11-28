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

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100'
  cell_level = 0

  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      block = '100'

      check_tally_sum = false
    []
    [Cell_2]
      type = CellTally
      block = '100'
      score = 'heating'

      check_tally_sum = false
    []
    [Mesh]
      type = MeshTally
      name = 'mesh_kf'
      mesh_template = '../../neutronics/meshes/sphere.e'
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'

      check_tally_sum = false
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_rel_err]
    type = TallyRelativeError
    tally_score = 'kappa_fission'
    value_type = max
  []
  [min_rel_err]
    type = TallyRelativeError
    tally_score = 'kappa_fission'
    value_type = min
  []
  [avg_rel_err]
    type = TallyRelativeError
    tally_score = 'kappa_fission'
    value_type = average
  []
[]

[Outputs]
  csv = true
[]
