[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 6
  theta_res = 8
  height = 0.008
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [vol_avg]
    family = MONOMIAL
    order = CONSTANT
  []
  [vol_integral]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Postprocessors]
  # we can compare the volumes computed on each block according to the binning
  # with near-analytic results from the subchannel mesh
  [vol_interior_all]
    type = VolumePostprocessor
    block = '1'
  []
  [vol_interior_channel]
    type = LinearCombinationPostprocessor
    pp_names = 'vol_interior_all'
    pp_coefs = '${fparse 1.0 / 6.0}'
  []
  [vol_edge_all]
    type = VolumePostprocessor
    block = '2'
  []
  [vol_edge_channel]
    type = LinearCombinationPostprocessor
    pp_names = 'vol_edge_all'
    pp_coefs = '${fparse 1.0 / 6.0}'
  []
  [vol_corner_all]
    type = VolumePostprocessor
    block = '3'
  []
  [vol_corner_channel]
    type = LinearCombinationPostprocessor
    pp_names = 'vol_corner_all'
    pp_coefs = '${fparse 1.0 / 6.0}'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  hide = 'vol_interior_all vol_edge_all vol_corner_all'
[]
