[Mesh]
  type = MeshGeneratorMesh
[]

[MeshGenerators]
  [file]
    type = FileMeshGenerator
    file = sphere.e
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'file'
    positions = '0 0 0 0 0 3.15'
  []
[]

[Outputs]
  exodus = true
[]
