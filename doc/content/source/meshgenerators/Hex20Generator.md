# Hex20Generator

!syntax description /Mesh/Hex20Generator

## Description

This mesh generator is used to convert a HEX27 mesh into a HEX20 mesh, specifically
for compatibility with NekRS's [exo2nek](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#:~:text=converting%20an%20exodus%20mesh) utility that generates a NekRS mesh (the custom
`.re2` format). This mesh generator will:

- Take a HEX27 mesh and convert it into an equivalent HEX20 mesh
- *Optionally* move nodes to be conformal with a cylinder. NekRS uses a high-order
  spectral element solution, which will exactly represent the cylinder surface with
  its [!ac](GLL) points (as long as the HEX20 elements have their mid-side nodes
  one the cylinder, which is what this mesh generator does for you)

!alert! note title="Have a HEX8 mesh instead of a HEX27 mesh?

If you have a HEX8 mesh, you can convert it to a HEX27 mesh by adding
`second_order = true` in a separate MOOSE file that generates your mesh
(which can be as simple as just reading the mesh from a file with a
[FileMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/FileMeshGenerator.html), like this

```
[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh.e
  []

  second_order = true
[]
```

Just for illustration, if you named the above as `mesh.i`, you can then run
`cardinal-opt -i mesh.i --mesh-only` to get a new mesh, `mesh_in.e` that is a HEX27 mesh. Then,
you can use this mesh generator.

!alert-end!

!alert warning
This mesh generator has very limited error checking, and will not protect you
from inverted elements or other errors. Always be sure to check your mesh output
for correct behavior.

## Example

This example converts from a HEX27 mesh to a HEX20 mesh, while also moving the outer boundary
(`rmax`) to a cylinder with radius of 0.25.

!listing /test/tests/meshgenerators/second_order_hex_generator/convert_outer.i

[rebuilt_mesh] shows the meshes involved in this example; the original mesh
(`tube_in.e`) is a HEX27 mesh, which gets converted to a HEX20 mesh (and we also
moved one boundary to a cylinder surface). Then, the [!ac](GLL) quadrature points
used in NekRS for a 5th order spectral element polynomial basis is shown on the right,
which by having the curved HEX20 elements, moves the quadrature points to also be
on the cylinder.

!media hex_generator.png
  id=rebuilt_mesh
  caption=Input, output, and NekRS meshes involved in mesh conversion example
  style=width:60%;margin-left:auto;margin-right:auto

For a second example, consider the case where you have one boundary that you
want to move with *multiple* origins. In this case, list the points in `origins`
for each boundary, with a semicolon separating the origins to be used for each
`boundary`.

!listing /test/tests/meshgenerators/second_order_hex_generator/multiple_origins.i

[rebuilt_mesh2] shows the meshes involved in this second example; the original
mesh (`fluid.exo`) is a HEX27 mesh, which gets converted to a HEX20 mesh where
boundary `1` is moved according to the nearest origin from a set of 7 different origins.
This allows you to group all the surfaces into one sideset, but move them to
unique origins.

!media multi_hex_generator.png
  id=rebuilt_mesh2
  caption=Input and output meshes when using multiple origins for a single sideset
  style=width:50%;margin-left:auto;margin-right:auto

When your mesh has boundary layers that are thin relative to the mesh movement
needed to fit to cylinder surfaces, it is possible to obtain inverted elements.
You can move the elements on the "other" side of the element face lying on the
cylinder by setting the `layers` variable. With this parameter, if node
$A$ on face 0 moves by $\Delta x$ to get onto the cylinder surface, then the
same displacement is applied to that node's "pair" on the opposite face.
An example of this usage is shown below, where nodes not on the boundary of
interest are still moved in order to better mesh the boundary layer.
[rebuilt_mesh3] shows the effect of setting `layers` (which defaults to zero)
to `1`.

!listing /test/tests/meshgenerators/second_order_hex_generator/layers.i

!media hex_generator_layers.png
  id=rebuilt_mesh3
  caption=Input and output meshes when using different `layers` settings
  style=width:90%;margin-left:auto;margin-right:auto

You can also set the `layers` to be greater than 1, in which case multiple
layers of boundary elements are moved. An example of this usage is shown below,
where elements 3 layers deep are moved to match the cylinder surface.
[rebuilt_mesh4] shows the effect of setting `layers` (which defaults to zero)
to `3`.

!listing test/tests/meshgenerators/second_order_hex_generator/three_layers.i

!media hex_generator_layers3.png
  id=rebuilt_mesh4
  caption=Input and output meshes when using `layers = 3`
  style=width:90%;margin-left:auto;margin-right:auto

!syntax parameters /Mesh/Hex20Generator

!syntax inputs /Mesh/Hex20Generator

!syntax children /Mesh/Hex20Generator
