# Generating CFD Meshes with MOOSE

There are several different ways to generate meshes for NekRS - you can use
Cubit, Gmsh, or another meshing utility. You can also directly use MOOSE
[MeshGenerators](https://mooseframework.inl.gov/source/meshgenerators/MeshGenerator.html)
to programmatically create meshes with MOOSE syntax.
Here, we have collected a number of meshing scripts we have developed that you
may find a useful starting point for MOOSE-based meshing.

- [Interassembly flow](interassembly.md)
- [Assembly flow](assembly.md)
- [Pin lattices](pins.md)
