num_layers = 1

channel_diameter = 0.016                 # diameter of the coolant channels (m)
height = 6.343                           # height of the full core (m)

[Mesh]
  # mesh mirror for the solid regions
  [solid]
    type = FileMeshGenerator
    file = solid_mesh_in.e
  []

  # create a mesh for a single coolant channel; because we will receive uniform
  # temperatures and densities on each x-y plane, we can use a very coarse
  # mesh in the radial direction
  [coolant_face]
    type = AnnularMeshGenerator
    nr = 1
    nt = 8
    rmin = 0.0
    rmax = ${fparse channel_diameter / 2.0}
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = coolant_face
    num_layers = ${num_layers}
    direction = '0 0 1'
    heights = '${height}'
    top_boundary = '300' # inlet
    bottom_boundary = '400' # outlet
  []
  [rename]
    type = RenameBlockGenerator
    input = extrude
    old_block = '1'
    new_block = '101'
  []

  # repeat the coolant channels and then combine together to get a combined mesh mirror
  [repeat]
    type = CombinerGenerator
    inputs = rename
    positions_file = coolant_channel_positions.txt
  []
  [add]
    type = CombinerGenerator
    inputs = 'solid repeat'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  initial_properties = 'xml'

  power = 1000.0
  scaling = 100.0
  temperature_blocks = '1 2 101'
  density_blocks = '101'
  cell_level = 1

  symmetry_mapper = sym
[]

[UserObjects]
  [sym]
    type = SymmetryPointGenerator
    normal = '${fparse -sqrt(3.0) / 2.0} 0.5 0.0'
  []
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []

  # check a few of the pins to be sure that the reflected heat source matches
  [pin1_l]
    type = PointValue
    variable = kappa_fission
    point = '0.0096 0.0489 4.0'
  []
  [pin1_r]
    type = PointValue
    variable = kappa_fission
    point = '0.0389 0.0326 4.0'
  []
  [diff1]
    type = DifferencePostprocessor
    value1 = pin1_l
    value2 = pin1_r
  []

  [pin2_l]
    type = PointValue
    variable = kappa_fission
    point = '-0.019 0.0329 4.0'
  []
  [pin2_r]
    type = PointValue
    variable = kappa_fission
    point = '0.0363 0.0 4.0'
  []
  [diff2]
    type = DifferencePostprocessor
    value1 = pin2_l
    value2 = pin2_r
  []

  [pin3_l]
    type = PointValue
    variable = kappa_fission
    point = '0.0463 0.115 4.0'
  []
  [pin3_r]
    type = PointValue
    variable = kappa_fission
    point = '0.0770 0.098 4.0'
  []
  [diff3]
    type = DifferencePostprocessor
    value1 = pin3_l
    value2 = pin3_r
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  csv = true

  hide = 'pin1_l pin1_r pin2_l pin2_r pin3_l pin3_r'
[]
