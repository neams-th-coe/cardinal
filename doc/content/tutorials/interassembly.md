# Interassembly Flow in Fast Reactors

The `utils/meshes/interassembly/mesh.py` script generates a HEX20
mesh suitable for fluid flow in the interassembly space outside the
ducts in a fast reactor. To create the mesh:

```
cd utils/meshes/interassembly/mesh.py
python mesh.py -g
```

You can control the mesh settings (numbers of elements, boundary layer
meshing strategy, etc.) and geometry (vessel diameter, duct thickness, etc.)
by directly editing the script parameters.

!listing utils/meshes/interassembly/mesh.py

An image of an example mesh is shown below.

!media interassembly_mesh.png
  id=interassembly_mesh
  caption=Example mesh which can be generated with the `interassembly` meshing script
  style=width:80%;margin-left:auto;margin-right:auto;halign:center
