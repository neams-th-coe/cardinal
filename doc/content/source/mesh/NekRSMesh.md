# NekRSMesh

!syntax description /Mesh/NekRSMesh

## Description

This class constructs a "mirror" mesh of the NekRS domain as a
[MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html) so that
other MOOSE applications can send/receive data to/from this mesh using
MOOSE [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html).
When data is *read* from NekRS, [NekRSProblem](/problems/NekRSProblem.md) directly
accesses solution arrays in NekRS and interpolates them onto the `NekRSMesh` mirror. When
data is *written* into NekRS, it is read from a `MooseVariable` on the `NekRSMesh`
and interpolated directly into boundary condition and source arrays in NekRS.

A mesh mirror can be constructed for

- Just the NekRS boundaries. This can be used when only boundary coupling/data is desired, such
  as for [!ac](CHT).
- The entire NekRS volume. This can be used when only volume coupling/data is desired, such
  as for temperature and density coupling to MOOSE.
- The entire NekRS volume *and* boundaries. This is the most general mirror,
  and can be used for concurrent boundary *and*/or volume
  coupled, such as for combined [!ac](CHT) and volume coupling to BISON and OpenMC, respectively.

For each choice of mesh domain, there are three options for the "resolution" of the mesh mirror:

- first order, or HEX8 elements (for volume mirrors) or QUAD4 elements (for boundary mirrors)
- second order, or HEX27 elements (for volume mirrors) or QUAD elements (for boundary mirrors)
- "exact" order, which will represent the NekRS high-order mesh exactly using HEX8 elements (for volume mirrors) or QUAD4 elements (for boundary mirrors).

### Boundary Mesh Mirror

In boundary mode, `NekRSMesh` will read the nodes associated with
all NekRS elements on a set of specified boundaries and
then build an equivalent `MooseMesh` of either first or second order. For example, suppose your NekRS
mesh is the one shown in [mesh1]; a boundary with ID 2 is indicated.
The mesh lines in [mesh1] are shown
connecting the [!ac](GLL) quadrature points in NekRS (*not* the eight nodes that would be needed to
define the corners of a hexahedral element).

!media mesh1.png
  id=mesh1
  caption=Example NekRS mesh; mesh lines are shown connecting [!ac](GLL) points
  style=width:40%;margin-left:auto;margin-right:auto

The three different types of mesh mirrors are shown in [mesh1_surface]. White dots
are shown to indicate the nodes for the first two mesh mirrors (too many nodes would
clutter the image if we showed the nodes for the exact mesh mirror as well).

!media mesh1_surface.png
  id=mesh1_surface
  caption=Three different mesh mirrors for boundary 2 in the NekRS mesh from [mesh1]
  style=width:100%;margin-left:auto;margin-right:auto

- First-order mesh mirrors are selected with `order = FIRST`, giving the
  first mesh mirror in [mesh1_surface]. NekRS's boundary elements are converted to QUAD4.
  That is, if NekRS uses a polynomial order of 7, data transfers between the
  mesh mirror and NekRS's mesh would involve interpolations on each element between
  $(7+1)^2$ NekRS quadrature points and 4 MOOSE nodes.
- Second-order mesh mirrors are selected with `order = SECOND`, giving the second mesh
  mirror in [mesh1_surface]. NekRS's boundary elements are converted to QUAD9.
  That is, if NekRS uses a polynomial order of 7, data
  transfers between the mesh mirror and NekRS's mesh would involve interpolations on each
  element between $(7+1)^2$ NekRS quadrature points and 9 MOOSE nodes.
- An "exact" mesh mirror is selected with `exact = true`, giving the third mesh mirror
  in [mesh1_surface]. For each NekRS boundary element, we build $N^2$ QUAD4 elements.

The boundary IDs in `boundary` correspond to the boundary IDs in the NekRS mesh,
i.e. in the `.re2` file.
You can set multiple boundaries to construct by passing a vector of sideset IDs;
this will add multiple boundaries into the same mirror mesh.

!listing test/tests/postprocessors/nek_weighted_side_integral/nek.i
  block=Mesh

!alert warning
When a boundary mesh mirror is constructed using `NekRSMesh`, the actual sidesets
associated with the original mesh are lost. For instance, setting `boundary = '1 2'`
will construct a mesh mirror
consisting of the elements on boundaries 1 and 2 in the NekRS mesh,
but those boundary IDs are not preserved in the mirror. This is a limitation of libMesh,
because *sideset* names cannot be associated with 2-D elements - only with
faces of 3-D elements. However, this has not yet
been a limitation in Cardinal, because we can still perform operations based on NekRS
sideset IDs through the various Nek postprocessors in Cardinal.
If these postprocessors do not meet your needs, you can retain NekRs sideset IDs when
constructing a volume mesh; please see [#v] and [#vb] for details.

### Volume Mesh Mirror
  id=v

In volume mode, `NekRSMesh` will
read the nodes associated with the entire NekRS mesh and then build an equivalent
`MooseMesh`. Again using the [mesh1] as an example NekRS mesh,
the three different types of mesh mirrors are shown in [mesh1_volume]. White dots
are shown to indicate the nodes for the first two mesh mirrors (too many nodes would
clutter the image if we showed the nodes for the exact mesh mirror as well).

!media mesh1_volume.png
  id=mesh1_volume
  caption=Three different mesh mirrors for the entire volume of the NekRS mesh from [mesh1]
  style=width:100%;margin-left:auto;margin-right:auto

- First-order mesh mirrors are selected with `order = FIRST`, giving the
  first mesh mirror in [mesh1_volume]. NekRS's elements are converted to HEX8.
  That is, if NekRS uses a polynomial order of 7, data transfers between the
  mesh mirror and NekRS's mesh would involve interpolations on each element between
  $(7+1)^3$ NekRS quadrature points and 8 MOOSE nodes.
- Second-order mesh mirrors are selected with `order = SECOND`, giving the second mesh
  mirror in [mesh1_volume]. NekRS's elements are converted to HEX27.
  That is, if NekRS uses a polynomial order of 7, data
  transfers between the mesh mirror and NekRS's mesh would involve interpolations on each
  element between $(7+1)^3$ NekRS quadrature points and 27 MOOSE nodes.
- An "exact" mesh mirror is selected with `exact = true`, giving the third mesh mirror
  in [mesh1_volume]. For each NekRS element, we build $N^3$ HEX8 elements.

If creating a mesh mirror in volume mode, the sideset IDs from the NekRS mesh
will be constructed in the mesh mirror.

### Volume and Boundary Mesh Mirrors
  id=vb

Combined volume and boundary mesh mirrors are used for:

- Combined [!ac](CHT) and volume coupling of NekRS to MOOSE

Combining the volume and boundary modes is achieved by setting both
`volume = true` and specifying the [!ac](CHT) boundaries with the `boundary` parameter.
The resulting volume mesh mirror will contain all the sidesets of the NekRS mesh
(because as described in [#v], all sidesets are automatically created in volume
mesh mirrors), and also set up necessary mapping data structures to facilitate
[!ac](CHT) to the NekRS boundaries specified with the `boundary` parameter.

For instance,
the following code snippet will generate a second-order volume mesh and sideset
2 will be coupled via [!ac](CHT) to MOOSE.

!listing
[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  boundary = '2'
[]

## Other Features

When NekRS is run in non-dimensional form, the `.re2` mesh will also be in non-dimensional
units. In this case, to ensure that the mesh mirror that handles data transfers to/from
*dimensional* coupled MOOSE applications (such as BISON) can correctly map the data in
space, a `scaling` factor must be applied to the mesh mirror.

For example, consider a hypothetical case where NekRS solves for
flow in a domain of size 5 m $\times$ 5 m. Suppose that the characteristic length is
taken as $L_{ref}=5$ m; in this case, the mesh in the `.re2` file should be of size
$1\times1$ to properly set up the nondimensional formulation. When the mesh mirror is constructed,
if no adjustments are made, the mesh mirror will extend over a range of $1\times1$ - but will
be coupled to a MOOSE application that probably selects a length scale as meters - not a nondimensional
scale. So, to convert the mesh mirror back to dimensional units, the `scaling` parameter
is used. For this example, if the coupled MOOSE application solves in units of meters,
then `scaling` should be set to 5 to dimensionalize the non-dimensional NekRS mesh.
Generally, `scaling` should be set to the same value used to "scale" the mesh when
using the `exo2nek` program.

!syntax parameters /Mesh/NekRSMesh

!syntax inputs /Mesh/NekRSMesh

!syntax children /Mesh/NekRSMesh
