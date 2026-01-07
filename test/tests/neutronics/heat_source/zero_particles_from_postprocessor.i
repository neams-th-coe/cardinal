[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Functions]
  [nparticles_fn]
    type = ParsedFunction
    value = 0
  []
[]

[Postprocessors]
  [n_particles]
    type = FunctionValuePostprocessor
    function = nparticles_fn
    execute_on = 'timestep_begin'
  []
  [particles_reporter]
    type = OpenMCParticles
    value_type = instantaneous
  []
  [total_particles_reporter]
    type = OpenMCParticles
    value_type = total
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100
  particles = n_particles

  verbose = true
  cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
  exodus = false
[]
