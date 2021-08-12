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
  [convert_to_m]
    type = TransformGenerator
    input = solid_ids
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
  solid_blocks = '100'
  skip_first_incoming_transfer = true
  verbose = true
  solid_cell_level = 0

  tally_type = mesh
  power = 100.0
  check_zero_tallies = false

  # the [Mesh] is in units of meters - VERY IMPORTANT: the mesh template still must be in
  # units of centimeters based on the constructors in OpenMC. We verify that this implementation
  # is correct if the power integral is 100 W, because the [Mesh] is in units of meters.
  scaling = 100.0
  mesh_template = '../meshes/sphere.e'
  normalize_by_global_tally = false
[]

[Executioner]
  type = Transient
  num_steps = 1

  # The quadrature rule used for integrating in 'heat_source' postprocessor
  # doesnt match the order for the problem if theres no nonlinear variables,
  # so we set the quadrature order here manually. Normally, OpenMCs heat source
  # is sent to another MOOSE app, which via a conservative transfer can be used
  # to ensure conservation.
  [Quadrature]
    type = GAUSS
    order = THIRD
  []
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
