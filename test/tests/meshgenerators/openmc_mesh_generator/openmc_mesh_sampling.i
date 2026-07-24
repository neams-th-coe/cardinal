[Mesh]
  [openmc_mesh]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 10
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  solve = false
[]

[AuxVariables]
  [elem_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [xcoord]
  []
  [ycoord]
  []
  [zcoord]
  []
[]

[AuxKernels]
  [elem_id_ak]
    type = ElementIDAux
    variable = elem_id
  []
  [xcoord_ak]
    type = FunctionAux
    variable = xcoord
    function = "x"
  []
  [ycoord_ak]
    type = FunctionAux
    variable = ycoord
    function = "y"
  []
  [zcoord_ak]
    type = FunctionAux
    variable = zcoord
    function = "z"
  []
[]

[VectorPostprocessors]
  [sample_points]
    type = PointValueSampler
    variable = "elem_id"
    points = "0.25 0.25 0.25
              0.75 0.25 0.25
              0.25 0.75 0.25
              0.75 0.75 0.25
              0.25 0.25 0.75
              0.75 0.25 0.75
              0.25 0.75 0.75
              0.75 0.75 0.75"
    sort_by = id
  []
[]

[Postprocessors]
  [total_volume]
    type = VolumePostprocessor
  []
  [num_elems]
    type = NumElements
  []
  [num_nodes]
    type = NumNodes
  []
  [x_min]
    type = NodalExtremeValue
    variable = xcoord
    value_type = min
  []
  [x_max]
    type = NodalExtremeValue
    variable = xcoord
    value_type = max
  []
  [y_min]
    type = NodalExtremeValue
    variable = ycoord
    value_type = min
  []
  [y_max]
    type = NodalExtremeValue
    variable = ycoord
    value_type = max
  []
  [z_min]
    type = NodalExtremeValue
    variable = zcoord
    value_type = min
  []
  [z_max]
    type = NodalExtremeValue
    variable = zcoord
    value_type = max
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
