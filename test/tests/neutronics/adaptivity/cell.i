[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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

[Adaptivity]
  steps = 1
  marker = uniform

  [Markers/uniform]
    type = UniformMarker
    mark = refine
  []
[]

[AuxVariables]
  [cell_volume]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_volume]
    type = CellVolumeAux
    variable = cell_volume
    volume_type = mapped
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  normalize_by_global_tally = false

  [Tallies]
    [Cell]
      type = CellTally
      score = kappa_fission
      blocks = '100'
    []
  []
[]

[Postprocessors]
  [Pebble_1_Vol]
    type = PointValue
    point = '0 0 0'
    variable = cell_volume
  []
  [Pebble_2_Vol]
    type = PointValue
    point = '0 0 4'
    variable = cell_volume
  []
  [Pebble_3_Vol]
    type = PointValue
    point = '0 0 8'
    variable = cell_volume
  []

  [Pebble_1_Heat]
    type = PointValue
    point = '0 0 0'
    variable = kappa_fission
  []
  [Pebble_2_Heat]
    type = PointValue
    point = '0 0 4'
    variable = kappa_fission
  []
  [Pebble_3_Heat]
    type = PointValue
    point = '0 0 8'
    variable = kappa_fission
  []

  [Num_Elem]
    type = NumElements
    elem_filter = total
  []
  [Num_Active_Elem]
    type = NumElements
    elem_filter = active
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
