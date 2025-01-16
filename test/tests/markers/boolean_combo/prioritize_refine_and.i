[Mesh]
  [m]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  uniform_refine = 1
[]

[Adaptivity]
  marker = combo
  steps = 1

  [Markers]
    [combo]
      type = BooleanComboMarker
      refine_markers = 'box box2'
      coarsen_markers = 'box2 box3'
      boolean_operator = and
      priority = refinement
    []
    [box]
      type = BoxMarker
      bottom_left = '0.3 0.3 0'
      top_right = '0.6 0.6 0'
      inside = refine
      outside = do_nothing
    []
    [box2]
      type = BoxMarker
      bottom_left = '0.5 0.5 0'
      top_right = '0.8 0.8 0'
      inside = refine
      outside = coarsen
    []
    [box3]
      type = BoxMarker
      bottom_left = '0.7 0.7 0'
      top_right = '1.0 1.0 0'
      inside = coarsen
      outside = do_nothing
    []
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = PJFNK
[]

[Postprocessors]
  [Num_Active]
    type = NumElements
    elem_filter = active
  []
[]

[Outputs]
  csv = true
[]
