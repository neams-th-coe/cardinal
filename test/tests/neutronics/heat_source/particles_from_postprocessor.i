[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
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
    execute_on = 'initial timestep_begin'
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
