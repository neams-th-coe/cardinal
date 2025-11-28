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
  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  cell_level = 0

  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Cell]
      type = CellTally
      score = 'kappa_fission'
      name = 'cell_kappa_fission'
      block = '100 200'
      normalize_by_global_tally = true
    []
    [Mesh]
      type = MeshTally
      score = 'kappa_fission flux'
      name = 'mesh_kappa_fission mesh_flux'

      # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_mesh_flux]
    type = ElementExtremeValue
    variable = mesh_flux
    value_type = 'max'
  []
  [max_mesh_kf]
    type = ElementExtremeValue
    variable = mesh_kappa_fission
    value_type = 'max'
  []
  [max_cell_kf]
    type = ElementExtremeValue
    variable = cell_kappa_fission
    value_type = 'max'
  []
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
[]
