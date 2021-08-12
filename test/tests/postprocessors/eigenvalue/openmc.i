[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
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
  power = 100.0
  solid_blocks = '100'
  tally_blocks = '100'
  solid_cell_level = 0
  tally_type = cell
  tally_filter = cell
  check_tally_sum = false

  # This turns off the density and temperature update on the first syncSolutions;
  # this uses whatever temperature and densities are set in OpenMCs XML files for first step
  skip_first_incoming_transfer = true
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [k_collision]
    type = KEigenvalue
    value_type = 'collision'
  []
  [k_absorption]
    type = KEigenvalue
    value_type = 'absorption'
  []
  [k_tracklength]
    type = KEigenvalue
    value_type = 'tracklength'
  []
  [k_combined]
    type = KEigenvalue
    value_type = 'combined'
  []
[]

[Outputs]
  csv = true
[]
