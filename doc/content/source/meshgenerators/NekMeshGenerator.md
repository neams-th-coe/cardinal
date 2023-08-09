# NekMeshGenerator

!syntax description /Mesh/NekMeshGenerator

## Description

This mesh generator is used to convert:

- A HEX27 mesh into a HEX20 mesh, or
- A QUAD9 mesh into a QUAD8 mesh

These two element types (HEX20, QUAD8) are
compatible with NekRS's [exo2nek](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#:~:text=converting%20an%20exodus%20mesh) utility that generates a NekRS mesh (the custom
`.re2` format). This page is written for the HEX27 to HEX20 conversion,
but the QUAD9 to QUAD8 conversion features are identical, just executed in 2-D.
This mesh generator will:

- Take a HEX27 mesh and convert it into an equivalent HEX20 mesh
- *Optionally* move nodes to be conformal with a cylinder.
  You can also *optionally* move nodes at polygon corners to a cylindrical radius of curvature.
  This allows NekRS to exactly represent curved corners of meshes that represent
  regular polygons, such as for curving the corners of a mesh representing a
  six-sided duct. Set `geometry_type = cylinder` for either of these.
- *Optionally* move nodes to be conformal with a sphere surface.

NekRS uses a high-order
spectral element solution, which will exactly represent circular surfaces
(e.g. circles, cylinders, spheres)
its [!ac](GLL) points (as long as the HEX20 elements have their mid-side nodes
one the curve, which is what this mesh generator does for you).

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

## Cylinder Examples

All examples in this section use `geometry_type = cylinder`.
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

Finally, you can move the corners of your mesh to a specified radius of curvature.
In the example below, the original mesh has an outer boundary that matches
a regular six-sided polygon. We move the corners,
as well as attached boundary layers, to have a radius of curvature.

!listing test/tests/meshgenerators/polygon_corners/six_with_bl.i

!media hex_generator_corners.png
  id=rebuilt_mesh5
  caption=Input and output meshes when enforcing radius of curvature at polygon corners
  style=width:90%;margin-left:auto;margin-right:auto

## Sphere Example

All examples in this section use `geometry_type = sphere`.
For example, if you have a sphere of radius $r=0.5$, you can move the nodes on the sphere
surface to a new radius, say $r=0.6$.

!listing test/tests/meshgenerators/sphere/convert.i

!media sphere_convert.png
  id=sphere_convert
  caption=Input and output meshes when enforcing sphere radius
  style=width:60%;margin-left:auto;margin-right:auto

!syntax parameters /Mesh/NekMeshGenerator

!syntax inputs /Mesh/NekMeshGenerator

!syntax children /Mesh/NekMeshGenerator
