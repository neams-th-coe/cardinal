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

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '0'
  initial_properties = xml
  verbose = true
  cell_level = 0

  tally_type = mesh
  power = 100.0
  check_tally_sum = false

  # make the mistake of clearly using a totally different mesh
  mesh_template = '../../neutronics/meshes/pincell.e'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
