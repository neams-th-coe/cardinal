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
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 10.0
  cell_level = 0
  temperature_blocks = '1'
[]

[Executioner]
  type = Transient
[]
