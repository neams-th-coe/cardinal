# Interassembly Flow in Fast Reactors

The interassembly region in fast reactors is the fluid space
*outside* the ducts, but inside the reactor vessel. The following
scripts mesh this geometry with various approximations. With all
of these scripts,
you can control the mesh settings (numbers of elements, boundary layer
meshing strategy, etc.) and geometry (vessel diameter, duct thickness, etc.)
by directly editing the script parameters.

## No Structural Components

To generate a mesh for interassembly flows without any structural components
(load pads, restraint rings, etc.) such as in [interassembly_mesh],
you can use the `utils/meshes/interassembly/mesh.py` script.

!media interassembly_mesh.png
  id=interassembly_mesh
  caption=Example mesh which can be generated with the `interassembly` meshing script
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

To create the mesh:

```
cd utils/meshes/interassembly/mesh.py
python mesh.py -g
```

## With Structural Components

To generate a mesh for interassembly flows with two sets of load pads
and restraint rings such as in [interassembly_mesh_structures],
you can use the `utils/meshes/interassembly_w_structures/mesh.py` script.

!media interassembly_mesh_structures.png
  id=interassembly_mesh_structures
  caption=Example mesh which can be generated with the `interassembly_w_structures` meshing script
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

To create the mesh:

```
cd utils/meshes/interassembly_w_structures/mesh.py
python mesh.py -g
```
