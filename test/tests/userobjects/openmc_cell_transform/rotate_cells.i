[Mesh]
  [pin]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '1.0'
    rings = '3 3'
    has_outer_square = on
    pitch = 4.0
    preserve_volumes = yes
    smoothing_max_it = 3
  []
  [assembly]
    type = CartesianIDPatternedMeshGenerator
    inputs = 'pin'
    pattern = '0 0;
               0 0'
    assign_type = 'cell'
    id_name = 'pin_id'
  []
  [delete]
    type = BlockDeletionGenerator
    input = assembly
    block = 2
  []
  [extrude]
    type = MeshExtruderGenerator
    input = delete
    extrusion_vector = '0 0 30'
  []
  [translate]
    type = TransformGenerator
    input = extrude
    transform = TRANSLATE_CENTER_ORIGIN
    vector_value = '-12 12 0'
  []
[]


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 2
  verbose = true
  power = 100
  fixed_mesh = false

  [Tallies]
    [heat]
      type = CellTally
      score = 'kappa_fission'
      block = '1'
    []
  []
[]

[AuxVariables]
  [disp_x]
    [AuxKernel]
        type = FunctionAux
        variable = disp_x
        function = disp_x_fn
        execute_on = 'timestep_begin'
    []
  []
  [disp_y]
    [AuxKernel]
        type = FunctionAux
        variable = disp_y
        function = disp_y_fn
        execute_on = 'timestep_begin'
    []
  []
  [disp_z]
  []
[]

[Functions]
  [psi_fn]
    type = ParsedFunction
    expression = '10*t'
  []
  [disp_x_fn]
    type = ParsedFunction
    expression = (cos(10*t*3.141593/180)-1)*x-sin(10*t*3.141593/180)*y
  []
  [disp_y_fn]
    type = ParsedFunction
    expression = sin(10*t*3.141593/180)*x+(cos(10*t*3.141593/180)-1)*y
  []
[]

[Postprocessors]
  [phi]
    type = ConstantPostprocessor
    value = 0.0
    execute_on = 'timestep_begin'
  []
  [theta]
    type = ConstantPostprocessor
    value = 0.0
    execute_on = 'timestep_begin'
  []
  [psi]
    type = FunctionValuePostprocessor
    function = psi_fn
    execute_on = 'timestep_begin'
  []
  [k]
    type = KEigenvalue
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '1'
  []
[]

[UserObjects]
  [translate_cells]
    type = OpenMCCellTransform
    transform_type = 'rotation'
    transform_array = 'phi theta psi'
    cell_ids = '2011'
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
