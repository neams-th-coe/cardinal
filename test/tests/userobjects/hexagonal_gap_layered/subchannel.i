[Mesh]
  type = HexagonalSubchannelGapMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 6
  height = 0.008
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
  [temp]
  []
[]

[Postprocessors]
  # we can compare the areas computed on each block according to the binning
  # with near-analytic results from the subchannel mesh
  [area_interior_all]
    type = VolumePostprocessor
    block = '1'
  []
  [area_interior] # area per gap (12 interior, 6 axial slices)
    type = LinearCombinationPostprocessor
    pp_names = 'area_interior_all'
    pp_coefs = '${fparse 1.0 / 12.0 / 6.0}'
  []
  [area_periph_all]
    type = VolumePostprocessor
    block = '2'
  []
  [area_periph] # area per gap (12 peripheral, 6 axial slices)
    type = LinearCombinationPostprocessor
    pp_names = 'area_periph_all'
    pp_coefs = '${fparse 1.0 / 12.0 / 6.0}'
  []

  # we can compare the average values computed on each block according to the binning
  # with near-analytic results from the subchannel mesh
  [avg_interior]
    type = ElementAverageValue
    variable = temp
    block = '1'
  []
  [avg_periph]
    type = ElementAverageValue
    variable = temp
    block = '2'
  []

  [area_interior_nek]
    type = ElementAverageValue
    variable = gap_area
    block = '1'
  []
  [area_periph_nek]
    type = ElementAverageValue
    variable = gap_area
    block = '2'
  []
  [avg_interior_nek]
    type = ElementAverageValue
    variable = gap_avg
    block = '1'
  []
  [avg_periph_nek]
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
  hide = 'area_interior_all area_periph_all'
[]
