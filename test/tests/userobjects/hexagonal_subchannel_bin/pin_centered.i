bundle_pitch = 2.5
pin_pitch = 0.8
pin_diameter = 0.6

[Mesh]
  [graphite_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse pin_pitch / 2.0}
    num_sectors_per_side = '2 2 2 2 2 2'
  []
  [bundle]
    type = PatternedHexMeshGenerator
    inputs = 'graphite_pin'
    hexagon_size = ${fparse bundle_pitch / 2.0}
    pattern = '0 0;
              0 0 0;
               0 0'
    rotate_angle = 0
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = bundle
    heights = 0.5
    num_layers = 5
    direction = '0 0 1'
  []
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [bin]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [bin]
    type = SpatialUserObjectAux
    variable = bin
    user_object = subchannel_bins
  []
[]

[UserObjects]
  [subchannel_bins]
    type = HexagonalSubchannelBin
    bundle_pitch = ${bundle_pitch}
    pin_pitch = ${pin_pitch}
    pin_diameter = ${pin_diameter}
    n_rings = 2
    pin_centered_bins = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
