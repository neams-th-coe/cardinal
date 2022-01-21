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
  check_tally_sum = false

  # this outputs the fission tally standard deviation in space
  output = 'fission_tally_std_dev'

  # This turns off the density and temperature update on the first syncSolutions;
  # this uses whatever temperature and densities are set in OpenMCs XML files for first step
  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_rel_err]
    type = FissionTallyRelativeError
    value_type = max
  []
  [min_rel_err]
    type = FissionTallyRelativeError
    value_type = min
  []
  [power_1]
    type = PointValue
    variable = heat_source
    point = '0 0 0'
  []
  [power_2]
    type = PointValue
    variable = heat_source
    point = '0 0 4'
  []
  [power_3]
    type = PointValue
    variable = heat_source
    point = '0 0 8'
  []

  [std_dev_1]
    type = PointValue
    variable = fission_tally_std_dev
    point = '0 0 0'
  []
  [std_dev_2]
    type = PointValue
    variable = fission_tally_std_dev
    point = '0 0 4'
  []
  [std_dev_3]
    type = PointValue
    variable = fission_tally_std_dev
    point = '0 0 8'
  []
[]

[Outputs]
  csv = true
[]
