[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Functions]
  [nparticles_fn]
    type = ParsedFunction
    value = 100*t
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
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
  exodus = false
[]
