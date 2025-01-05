# HexagonalSubchannelGapMesh

!syntax description /Mesh/HexagonalSubchannelGapMesh

## Description

This class builds a 2-D mesh of the gap planes in a
triangular lattice of pins enclosed in a hexagonal duct with a uniform axial
discretization.

To allow easy averages or other postprocessors to be applied by gap type,
all interior gaps are by default set to a subdomain ID of 1, while all peripheral
gaps are set to a subdomain ID of 2. The `interior_id` and `peripheral_id` parameters
can be used to control this behavior.

!alert warning
This class is intended *ONLY* for visualization purposes - node connectivity between
elements is not obeyed, so you cannot use this mesh to solve any anything that requires
connectivity information between elements (such as a finite element solve).

## Example Input syntax

The following shows the mesh generated for a bundle with three rings of pins
with ten axial layers.

!listing test/tests/mesh/hexagonal_gap_mesh/three_rings.i
  block=Mesh

An image of the generated mesh is shown in [tri_mesh]. The elements
are colored by block ID. By default, all
interior channels are added to block 1, while all peripheral gaps are added to block 2.

!media media/tri_mesh_gaps.png
  id=tri_mesh
  caption=Gap mesh generated with three pin rings; elements are colored by block IDs
  style=width:60%;margin-left:auto;margin-right:auto

While the `HexagonalSubchannelGapMesh` does *not* generate meshes of any other regions except for the gaps,
it can be useful to display extra context/the shape of the assembly to better represent data.
To do so in [tri_mesh], we created a separate input file using a
[HexagonalSubchannelMesh](/mesh/HexagonalSubchannelMesh.md), with the same geometry
parameters as used above:

!listing test/tests/mesh/hexagonal_gap_mesh/context_mesh3.i

After running this mesh, we simply open `context_mesh3_out.e` in Paraview at
the same time that we have already loaded `three_rings_out.e` and slice along the $x$-$y$ midplane.

!syntax parameters /Mesh/HexagonalSubchannelGapMesh

!syntax inputs /Mesh/HexagonalSubchannelGapMesh
