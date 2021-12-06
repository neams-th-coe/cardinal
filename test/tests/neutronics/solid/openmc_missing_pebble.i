# This is a vertical stack of three pebbles, with a 1 meter thick layer of flibe
# on the outer periphery and on the top and bottom, to get a k closer to 1.0.
#
# We check that we get an error if we forget one of the pebbles.

[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [multiple]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4'
  []

  parallel_type = replicated
[]

[Problem]
  type = OpenMCProblem
  power = 10.0
  centers = '0 0 0
             0 0 4'
  volumes = '1.322128e+01'
  tally_type = 'cell'
  pebble_cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
