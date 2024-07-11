[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 100.0
  temperature_blocks = '100'
  cell_level = 1
  scaling = 100.0

  relaxation = dufek_gudowski
  first_iteration_particles = 500
  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [particles]
    type = OpenMCParticles
    value_type = instantaneous
  []
  [particles_total]
    type = OpenMCParticles
    value_type = total
  []
[]
