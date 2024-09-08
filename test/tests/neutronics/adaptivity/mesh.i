[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [solid]
    type = CombinerGenerator
    inputs = solid_ids
    positions = '0 0 0
                 0 0 4
                 0 0 8'
    avoid_merging_subdomains = true
  []

  allow_renumbering = false
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
  temperature_blocks = '100 101 102'
  cell_level = 0
  initial_properties = xml

  # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
  normalize_by_global_tally = false

  [Tallies]
    [Mesh]
      type = MeshTally
      score = kappa_fission
    []
  []
[]

[Postprocessors]
  [Pebble_1_Heat]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '100'
  []
  [Pebble_2_Heat]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '101'
  []
  [Pebble_3_Heat]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '102'
  []

  [Num_Elem]
    type = NumElements
    elem_filter = total
  []
  [Num_Active_Elem]
    type = NumElements
    elem_filter = active
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
