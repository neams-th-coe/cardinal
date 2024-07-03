# Using the Layered Gap Bin

On many occasions, getting planar integrals and averages from NekRS is necessary for postprocessing
the CFD data generated as part of a standalone Nek case, or a coupled multiphysics simulation within
Cardinal. This requires finding a coplanar set of points and performing numerical integration. In the
Nek mesh, as in most finite element codes, the 2D quadrature weights are defined exclusively on
element faces. This poses the following challenge: how does one perform an area integral or average
at an arbitrary plane that is not coincident any of the mesh faces?

There are several solutions in the Nek eco-system to address this problem. Cardinal offers a simple
and user-friendly way to perform such an operation, using the `LayeredGapBin`
[source documentation page](source/userobjects/LayeredGapBin.md) and `NekBinnedPlaneIntegral`
[source documentation page](source/userobjects/NekBinnedPlaneIntegral.md) userobjects.
The former works by dividing the mesh into coplanar bins along the x, y, or z axes. The
latter performs a volume integral over a thin layer of elements, and then divides it by the gap thickness to obtain an area integral. This approach works well for extruded meshes, which are very common. This
tutorial describes how to use these capabilities within Cardinal.


## Nek Case Setup

The average pressure over bins over `LayeredGapBins` defined along the z-axis will be calculated.
Since the focus is on postprocessing and no multiphysics capabilities are required, the tutorial
will stick to using the `NekRSStandaloneProblem` class.

To avoid versioning a large Nek mesh file, the Gmsh mesh file `mesh.geo` is provided
(`/tutorials/layered_gap_bin/mesh.geo`). As a useful aside, we describe the process of generating a
mesh from Gmsh and converting it to the Nek format using `gmsh2nek`. Open the `geo` file in Gmsh,
generate a 3D mesh, and select `Set Order 2`. Export the mesh as a Version 2 ASCII `msh` file. The given
file has sidesets 1 and 2 as wall boundaries, and sidesets 3-10 have periodic boundaries, with each
successive pair (3 and 4, 5 and 6...) forming a periodic pair. The following settings should be used
in `gmsh2nek` (see [table1]).

!table id=table1 caption=Gmsh input parameters
| Parameter | Value |
| :- | :- |
| Mesh dimension | 3 |
| Input .msh file name | User-defined |
| Solid mesh? | 0 (No) |
| Periodic surface pairs | 4 |
| Periodic pairs to input | 3 and 4 |
| :- | 5 and 6 |
| :- | 7 and 8 |
| :- | 9 and 10|
| `re2` name | User-defined |

The periodic sidesets must have their boundary IDs set to `0` in Nek to indicate that they are
"internal" (periodic) sidesets. This can be done in the `usr` file as follows

!listing /tutorials/layered_gap_bin/periodic_duct.usr
  start=      do iel=1,nelt ! removing periodic sidesets
  end=c     non-dimensionalising the mesh

## Layered Gap Bin Setup

The following code sets up 4 equidistant planar `LayeredGapBins`, and performs an area integration
over the fields `pressure` and `unity`.

!listing /tutorials/layered_gap_bin/nek.i
  block=UserObjects

The parameter `gap_thickness` is key. It should not be too small, or [!ac](GLL) points will not get
mapped to the bin. However, it should not be so large that it makes the `NekBinnedPlaneIntegral`
inaccurate. One way to verify and calibrate `gap_thickness` for extruded meshes is to compare the
area of the bin plane to a known area, such as the 2D geometry the mesh was extruded from. In this
mesh, that would be the inlet (sideset #3). Before its ID is set to 0, as discussed above, its area
can be calculate in Nek itself using the following code

!listing /tutorials/layered_gap_bin/periodic_duct.usr
  start=      ntot = lx1*ly1*lz1*nelt
  end= inlet_area/dh/dh

This allows the user to verify the `gap_thickness` is set to an appropriate value. If the integral of
`unity` does not result in the expected area value, the user should adjust
1. the `gap_thickness`: if the difference is small
2. verify that their non-dimensionalisation is consistent across the Nek (e.g. `usr`) and Cardinal
input files: if the difference is large

Then the fields such as the pressure can be used in either `NekBinnedPlaneIntegral` or `NekBinnedPlaneAverage`
for postprocessing needs such as calculating the pressure drop, analysing the impact of structural
components for reduced-order modelling etc.
