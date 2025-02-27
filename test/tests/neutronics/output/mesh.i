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
  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  source_strength = 1e6

  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0

  initial_properties = xml

  check_tally_sum = false

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'flux'
      output = 'unrelaxed_tally'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_flux]
    type = ElementIntegralVariablePostprocessor
    variable = flux_raw
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
