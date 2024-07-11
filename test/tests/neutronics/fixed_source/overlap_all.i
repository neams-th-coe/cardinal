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
    file = ../heat_source/stoplight.exo
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

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100 200'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  source_strength = 1e12
  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0
  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [heat_source_fluid]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '200'
  []
  [heat_source_solid]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '100'
  []
[]

[Outputs]
  csv = true
[]
