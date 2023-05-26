[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 4
    nz = 4
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 2.0
    bias_x = 1.2
    elem_type = HEX20
  []
  [rename]
    type = RenameBoundaryGenerator
    input = box
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '1 2 3 4 5 6'
  []
[]

# The following blocks only exist here for testing purposes, to have some
# postprocessors we check with a CSVDiff test to ensure this tutorial
# stays up to date with future possible syntax changes in MOOSE.

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [area_left]
    type = AreaPostprocessor
    boundary = '5'
  []
  [area_right]
    type = AreaPostprocessor
    boundary = '3'
  []
[]

[Outputs]
  csv = true
  file_base = channel
[]
