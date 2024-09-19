# Within-Duct Assembly Flow in Fast Reactors

The within-duct assembly region in fast reactors is the fluid space
surrounding the fuel pins, within the ducts. We have developed a script
to mesh this geometry, with control of the
mesh settings (numbers of elements, boundary layer
meshing strategy, etc.) and geometry (pin diameter, etc.)
by directly editing the script parameters.

## Bare Bundle

To generate a mesh for fluid flow without the wires such as in [assembly_mesh],
you can use the `utils/meshes/assembly/mesh.py` script.

!media assembly_mesh.png
  id=assembly_mesh
  caption=Example mesh which can be generated with the `assembly` meshing script
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

To create the mesh:

```
cd utils/meshes/assembly/mesh.py
python mesh.py -g
```

The mesh will be generated with the following sideset IDs:

 - 1: fuel pins
 - 2: inlet
 - 3: outlet
 - 4: hex can wall

