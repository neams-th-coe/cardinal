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
  # approximation of the true projected velocity which we compute from the velocity components
  # interpolated onto the first-order NekRSMesh mesh mirror
  [velocity_component]
  []

  # actual result of the user object, which should be close to 'velocity_component'
  [avg_velocity_component]
    family = MONOMIAL
    order = CONSTANT
  []

  # These are just for visualizing the average velocity component with Glyphs in paraview;
  # the result of the 'vol_avg' user object will be represented as a vector "uo_" with 3 components
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
