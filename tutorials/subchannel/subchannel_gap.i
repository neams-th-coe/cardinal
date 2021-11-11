[Mesh]
  type = HexagonalSubchannelGapMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 7
  height = 0.008
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [average_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_gap_velocity]
    family = MONOMIAL
    order = CONSTANT
  []

  # These are just for visualizing the average velocity component with Glyphs in paraview
  [uo_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo_z]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
