# Pin Lattices

The solid pins in fast reactors are cylinders spaced in hexagonal lattices.
We have developed a script
to mesh a bare version of this geometry, with control of the
mesh settings (numbers of elements, etc.)
and geometry (pin diameter, etc.)
by directly editing the script parameters.

## Bare Bundle

To generate a mesh for solid heat transfer without the wires such as in [pin_mesh],
you can use the `utils/meshes/fuel_pins/mesh.py` script.

!media pin_mesh.png
  id=pin_mesh
  caption=Example mesh which can be generated with the `fuel_pins` meshing script, shown for 7 assemblies each with 7 pins
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

To create the mesh:

```
cd utils/meshes/fuel_pins/mesh.py
python mesh.py -g
```
