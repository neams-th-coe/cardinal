[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere_in_m.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[Tallies]
  [Mesh]
    type = MeshTally
    # We incorrectly provide a mesh in units of centimeters
    mesh_template = '../../neutronics/meshes/sphere.e'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0
  check_tally_sum = false

  scaling = 100.0
  normalize_by_global_tally = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
