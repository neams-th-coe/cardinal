[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0'
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

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0

  initial_properties = xml

  verbose = true
  cell_level = 0

  [Tallies]
    [Cell]
      type = CellTally
      name = heat_source

      # This input has fissile material in the fluid phase, so we will get a warning
      # that we are neglecting some of the global kappa-fission distribution; so here
      # the power is the total power of the OpenMC problem, and only a fraction of this
      # will be computed in the solid pebbles
      block = '100'

      # We are skipping some feedback with fissile regions, so we need to turn off the check
      check_tally_sum = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [solid_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '200'
  []
[]
