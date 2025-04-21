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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'heating'

  calc_kinetics_params = true
  ifp_generations = 5

  [Tallies]
    [Cell]
      type = CellTally
      score = 'heating flux'
      blocks = '100 200'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [alpha]
    type = AlphaEffective
  []
  [beta]
    type = BetaEffective
  []
[]

[Outputs]
  csv = true
[]
