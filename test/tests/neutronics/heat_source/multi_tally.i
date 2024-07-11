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

[Tallies]
  [Cell]
    type = CellTally
    tally_score = 'kappa_fission heating'
    tally_blocks = '100 200'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100 200'
  density_blocks = '200'
  verbose = true
  cell_level = 0

  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [fluid_kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '200'
  []
  [solid_kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '100'
  []
  [heating]
    type = ElementIntegralVariablePostprocessor
    variable = heating
  []
  [fluid_heating]
    type = ElementIntegralVariablePostprocessor
    variable = heating
    block = '200'
  []
  [solid_heating]
    type = ElementIntegralVariablePostprocessor
    variable = heating
    block = '100'
  []
[]

[Outputs]
  csv = true
[]
