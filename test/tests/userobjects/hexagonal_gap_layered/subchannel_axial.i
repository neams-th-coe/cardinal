[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 6
  height = 0.008
  volume_mesh = false
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [gap_avg]
    family = MONOMIAL
    order = CONSTANT
  []
  [gap_area]
    family = MONOMIAL
    order = CONSTANT
  []
  [P]
  []
[]

[Postprocessors]
  # we can compare the average values computed on each block according to the binning
  # with near-analytic results from the subchannel mesh
  [avg_interior]
    type = ElementAverageValue
    variable = P
    block = '1'
  []
  [avg_edge]
    type = ElementAverageValue
    variable = P
    block = '2'
  []
  [avg_corner]
    type = ElementAverageValue
    variable = P
    block = '3'
  []

  [avg_interior_nek]
    type = ElementAverageValue
    variable = gap_avg
    block = '1'
  []
  [avg_edge_nek]
    type = ElementAverageValue
    variable = gap_avg
    block = '2'
  []
  [avg_corner_nek]
    type = ElementAverageValue
    variable = gap_avg
    block = '2'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
