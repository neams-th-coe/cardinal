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

[Postprocessors]
  [particles]
    type = ConstantPostprocessor
    value = 110
    execute_on = 'initial'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100
  particles = particles
  temperature_blocks = '100 200'
  verbose = true
  cell_level = 0
  batches = 50
  inactive_batches = 10

  [Tallies]
    [Cell]
      type = CellTally
      block = '100 200'
      name = heat_source
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [fluid_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '200'
  []
  [solid_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '100'
  []
[]

[Outputs]
  execute_on = final
  exodus = true
[]
