[Mesh]
  type = FileMesh
  file = mesh1_in.e
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [u]
  []
[]

[Functions]
  [u]
    type = ParsedFunction
    expression = 'exp(x)*exp(y)'
  []
[]

[AuxKernels]
  [u]
    type = FunctionAux
    variable = u
    function = u
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = 'sub.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [interpolation]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = u
    variable = u_interpolation
    to_multi_app = sub
  []
  [mesh_function]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = u
    variable = u_mesh_function
    to_multi_app = sub

    # uses extrapolation to set the local value at any points which are missed due to the
    # geometries not overlapping in some regions
    error_on_miss = false
  []
  [nearest_node]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = u
    variable = u_nearest_node
    to_multi_app = sub
  []
  [projection]
    type = MultiAppProjectionTransfer
    source_variable = u
    variable = u_projection
    to_multi_app = sub
  []
  [sample]
    type = MultiAppVariableValueSampleTransfer
    source_variable = u
    variable = u_sample
    to_multi_app = sub
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
