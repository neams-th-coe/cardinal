# HexagonalSubchannelMesh

!syntax description /Mesh/HexagonalSubchannelMesh

## Description

This class builds a 3-D mesh of a
triangular lattice of pins enclosed in a hexagonal duct with a uniform axial
discretization. The `theta_res` parameter indicates how many nodes should be
placed on the pin's surface in each channel, while the `gap_res` parameter
indicates how many nodes should be placed on each gap.

To allow easy averages or other postprocessors to be applied by channel type,
all interior, edge, and corner channels are by default set to a unique subdomain ID
(1 for the interior channels, 2 for the edge channels, and 3 for the corner channels).
This behavior can be controlled by setting the `interior_id`, `edge_id`, and
`corner_id` parameters to the desired subdomain IDs.

## Example Input syntax

The following shows the mesh generated for a bundle with three rings of pins
with two axial layers.

!listing test/tests/mesh/hexagonal_subchannel_mesh/three_rings.i
  block=Mesh

An image of the generated mesh is shown in [tri_mesh]. The elements
are colored by block ID. By default, all
interior channels are added to block 1, all edge channels to block 2,
and all corner channels to block 3.

!media media/tri_mesh.png
  id=tri_mesh
  caption=Mesh generated with three pin rings; elements are colored by block IDs
  style=width:60%;margin-left:auto;margin-right:auto

!syntax parameters /Mesh/HexagonalSubchannelMesh

!syntax inputs /Mesh/HexagonalSubchannelMesh

!syntax children /Mesh/HexagonalSubchannelMesh
