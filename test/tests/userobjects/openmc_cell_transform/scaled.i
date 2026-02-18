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
    # Only keeping block 1 (fuel pins) for this test
    type = BlockDeletionGenerator
    input = assembly
    block = 2
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete
    direction = '0 0 1'
    heights = '30'
    num_layers = 1
  []
  [translate]
    type = TransformGenerator
    input = extrude
    transform = TRANSLATE_CENTER_ORIGIN
    vector_value = '-12 12 0'
  []
  [scale_to_m]
    type = TransformGenerator
    input = translate
    transform = SCALE
    vector_value = '0.01 0.01 0.01'
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
  scaling = 100.0

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
  [disp_x_fn]
    type = ParsedFunction
    expression = '1e-2*2*t'
  []
  [disp_y_fn]
    type = ParsedFunction
    expression = '1e-2*2*t'
  []
[]

[Postprocessors]
  [shift_x]
    type = FunctionValuePostprocessor
    function = disp_x_fn
    execute_on = 'timestep_begin'
  []
  [shift_y]
    type = FunctionValuePostprocessor
    function = disp_y_fn
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
    transform_type = 'translation'
    vector_value = 'shift_x shift_y 0'
    cell_ids = '2011'
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
