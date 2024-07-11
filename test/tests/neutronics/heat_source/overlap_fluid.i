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
    tally_name = heat_source

    # This input has fissile material in the solid phase, so we will get a warning
    # that we are neglecting some of the global kappa-fission distribution; so here
    # the power is the total power of the OpenMC problem, and only a fraction of this
    # will be computed in the fluid
    tally_blocks = '200'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100 200'
  density_blocks = '200'

  initial_properties = xml

  # We are skipping some feedback with fissile regions, so we need to turn off the check
  check_tally_sum = false

  verbose = true
  cell_level = 0
[]

[Executioner]
  type = Steady
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
  exodus = true
  csv = true
  hide = 'density'
[]
