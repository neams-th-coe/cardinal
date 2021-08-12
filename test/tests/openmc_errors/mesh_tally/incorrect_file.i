[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [block]
    type = SubdomainIDGenerator
    input = sphere
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
  power = 100.0
  check_tally_sum = false
  check_zero_tallies = false

  # make the mistake of clearly using a totally different mesh
  mesh_template = '../../neutronics/meshes/pincell.e'
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
