[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [combine]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [block]
    type = SubdomainIDGenerator
    input = combine
    subdomain_id = '0'
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
  solid_blocks = '0'
  skip_first_incoming_transfer = true
  verbose = true
  solid_cell_level = 0

  tally_type = mesh
  mesh_template = '../../neutronics/meshes/sphere.e'
  power = 100.0
  check_tally_sum = false
  check_zero_tallies = false

  # Make the mistake of listing the mesh template translations in a different order than
  # was used to create the [Mesh]. This should catch that the mesh template does not
  # exactly match the [Mesh]
  mesh_translations = '0 0 0
                       0 0 8
                       0 0 4'
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
[]

[Outputs]
  exodus = true
  hide = 'dummy temp'
[]
