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

  # we need this to match the quadrature used in the receiving MOOSE app
  # (does not exist in this input file) so that the elem->volume() computed
  # for normalization within OpenMCCellAverageProblem is the same as in the
  # receiving MOOSE app.
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
