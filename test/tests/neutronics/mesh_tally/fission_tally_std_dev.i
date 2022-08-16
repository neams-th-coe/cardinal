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
  solid_blocks = '100'
  initial_properties = xml
  verbose = true
  solid_cell_level = 0
  normalize_by_global_tally = true

  tally_type = mesh
  mesh_template = '../meshes/sphere.e'
  power = 100.0
  check_tally_sum = false
  check_zero_tallies = false

  output = 'unrelaxed_tally_std_dev'
  output_name = 'fission_tally_std_dev'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heat_source_at_pt]
    type = PointValue
    variable = heat_source
    point = '0.896826 0.189852 0.608855' # centroid of first element in mesh
  []
  [std_dev_at_pt]
    type = PointValue
    variable = fission_tally_std_dev
    point = '0.896826 0.189852 0.608855'
  []
[]

[Outputs]
  csv = true
[]
