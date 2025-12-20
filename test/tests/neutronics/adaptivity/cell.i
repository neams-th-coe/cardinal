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
[]

[Adaptivity]
  steps = 1
  marker = uniform

  [Markers/uniform]
    type = UniformMarker
    mark = refine
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      score = kappa_fission
      block = '100'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  hide = 'temp cell_instance'
[]
