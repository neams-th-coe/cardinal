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

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = '../neutronics/meshes/sphere.e'
    mesh_translations_file = pebble_centers.txt
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
[]

[Executioner]
  type = Transient
[]
