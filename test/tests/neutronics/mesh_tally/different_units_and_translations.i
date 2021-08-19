[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [sphereb]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid2]
    type = SubdomainIDGenerator
    input = sphereb
    subdomain_id = '200'
  []
  [spherec]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid3]
    type = SubdomainIDGenerator
    input = spherec
    subdomain_id = '300'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid1 solid2 solid3'
    positions_file = pebble_centers.txt
  []
  [convert_to_m]
    type = TransformGenerator
    input = combine
    transform = scale
    vector_value = '0.01 0.01 0.01'
  []

  parallel_type = replicated
[]

# This AuxVariable and AuxKernel is only here to get the postprocessors
# to evaluate correctly. This can be deleted after MOOSE issue #17534 is fixed.
[AuxVariables]
  [dummy]
  []
[]

[AuxKernels]
  [dummy]
    type = ConstantAux
    variable = dummy
    value = 0.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  solid_blocks = '100 200 300'
  skip_first_incoming_transfer = true
  verbose = true
  solid_cell_level = 0
  normalize_by_global_tally = false

  # When using different units, the mesh translations must still be provided in centimeters
  mesh_translations_file = pebble_centers.txt

  tally_type = mesh

  # the [Mesh] is in units of meters - VERY IMPORTANT: the mesh template still must be in
  # units of centimeters based on the constructors in OpenMC. We verify that this implementation
  # is correct if the power integral is 100 W, because the [Mesh] is in units of meters.
  scaling = 100.0
  mesh_template = '../meshes/sphere.e'

  power = 100.0
  check_zero_tallies = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [heat_pebble1]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '100'
  []
  [heat_pebble2]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '200'
  []
  [heat_pebble3]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '300'
  []
[]

[Outputs]
  exodus = true
  hide = 'dummy temp'
[]
