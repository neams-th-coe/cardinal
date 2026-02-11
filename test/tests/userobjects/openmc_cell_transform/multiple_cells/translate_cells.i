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
    direction = '0 0 1'
    input = delete
    heights = '30'
    num_layers = '5'
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
        function = disp_right_fn
    []
  []
  [disp_y]
    [AuxKernel]
        type = FunctionAux
        variable = disp_y
        function = disp_top_fn
    []
  []
  [disp_z]
  []
[]

[Functions]
  [disp_top_fn]
    type = ParsedFunction
    expression = 'if(t>1,if(y>0,2*t,0),0)'
  []
  [disp_right_fn]
    type = ParsedFunction
    expression = 'if(t>1,if(x>0,t,0),0)'
  []
[]

[Postprocessors]
  [shift_top_row]
    type = FunctionValuePostprocessor
    function = disp_top_fn
  []
  [shift_right_column]
    type = FunctionValuePostprocessor
    function = disp_right_fn
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
  [translate_top]
    type = OpenMCCellTransform
    transform_type = 'translation'
    vector_value = '0 shift_top_row 0'
    cell_ids = '2013 2014'
  []
  [translate_right]
    type = OpenMCCellTransform
    transform_type = 'translation'
    vector_value = 'shift_right_column 0 0'
    cell_ids = '2012 2014'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
