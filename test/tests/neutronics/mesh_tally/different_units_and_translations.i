[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere_in_m.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [sphereb]
    type = FileMeshGenerator
    file = ../meshes/sphere_in_m.e
  []
  [solid2]
    type = SubdomainIDGenerator
    input = sphereb
    subdomain_id = '200'
  []
  [spherec]
    type = FileMeshGenerator
    file = ../meshes/sphere_in_m.e
  []
  [solid3]
    type = SubdomainIDGenerator
    input = spherec
    subdomain_id = '300'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid1 solid2 solid3'
    positions_file = pebble_centers_in_m.txt
  []

  allow_renumbering = false
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
  initial_properties = xml
  verbose = true
  solid_cell_level = 0
  normalize_by_global_tally = false

  tally_type = mesh

  # Because the [Mesh] is in units of meters, the mesh translations and mesh template
  # must also be in units of meters
  scaling = 100.0
  mesh_translations_file = pebble_centers_in_m.txt
  mesh_template = '../meshes/sphere_in_m.e'

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
