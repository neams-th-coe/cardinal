[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 6
  theta_res = 3
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
  [vol_avg]
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

  # unhide and turn the number of time steps in the .par file to greater than 1 in order to see this
  # match the user object
  hide = 'velocity_component'
[]
