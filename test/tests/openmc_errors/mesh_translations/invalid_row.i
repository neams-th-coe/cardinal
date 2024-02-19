[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [combine]
    type = CombinerGenerator
    inputs = 'sphere'
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  mesh_translations_file = pebble_centers.txt

  tally_type = mesh
  mesh_template = '../neutronics/meshes/sphere.e'
  power = 100.0
[]

[Executioner]
  type = Transient
[]
