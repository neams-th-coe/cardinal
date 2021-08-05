# NekRSMesh

!syntax description /Mesh/NekRSMesh

## Description

This class constructs a "mirror" mesh of the NekRS domain as a
[MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html) so that
other MOOSE applications can send/receive data to/from this mesh using standard
MOOSE [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html). In other
words, [NekRSProblem](/problems/NekRSProblem.md) reads/writes fields from/to NekRS.
When data is *read* from NekRS, `NekRSProblem` directly
accesses solution arrays in NekRS and interpolates them onto the `NekRSMesh` mirror
(specifically, into [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
initialized ahead of time by `NekRSProblem`). When
data is *written* into NekRS, it is read from a `MooseVariable` on the `NekRSMesh` mirror
and interpolated directly into boundary condition and source arrays in NekRS.

A mesh mirror can be constructed for

- Just the boundaries in a NekRS domain (such as for cases where only boundary coupling/data is desired)
- The entire NekRS volume (such as for cases where only volume coupling/data is desired)
- The entire NekRS volume *and* boundaries (such as for cases where both boundary and volume coupling/data is desired)

Each of these three modes are now described.

### Boundary Mesh Mirror

For applications with [!ac](CHT) coupling of NekRS to MOOSE, and/or for the more general
use case of projecting a NekRS solution on a *boundary* to another MOOSE application,
`NekRSMesh` should be used in *boundary* mode. This will read the nodes associated with
all NekRS elements on a set of specified boundaries, via the `boundary` parameter, and
then build an equivalent `MooseMesh` of either first or second order. For example, consider
the NekRS mesh shown in [mesh1]; a boundary with ID `2` is indicated. Mesh lines are shown
connecting the quadrature points in NekRS - *not* the eight nodes that would be needed to
define the corners of a hexahedral element.

!media mesh1.png
  id=mesh1
  caption=Example NekRS mesh; mesh lines are shown connecting [!ac](GLL) points
  style=width:60%;margin-left:auto;margin-right:auto

The code snippet below shows how to construct a first-order representation of boundary `2` as a `MooseMesh`.
We set the `order` of the mesh to `FIRST`, meaning that NekRS's elements are converted
to HEX8.

!listing /test/tests/conduction/nonidentical_interface/cylinders/nek.i
  id=mesh1_listing
  caption=Creation of first-order boundary mesh mirror
  block=Mesh

The mesh mirror produced by `NekRSMesh` is shown in [mesh1_mirror]. The four nodes
on each QUAD4 element are shown as white dots.

!media mesh1_mirror.png
  id=mesh1_mirror
  caption=First-order surface mesh mirror of the NekRS mesh shown in [mesh1], for sideset `2`.
  style=width:40%;margin-left:auto;margin-right:auto

By default, a first-order mesh is constructed. But you can also create a second-order
mesh by setting `order = SECOND`. If we change `order = FIRST` in
[mesh1_listing] to `order = SECOND`, the surface elements are now represented as QUAD9 elements,
as shown in [mesh1_mirrorb].

!media mesh1_mirrorb.png
  id=mesh1_mirrorb
  caption=Second-order surface mesh mirror of the NekRS mesh shown in [mesh1], for sideset `2`.
  style=width:40%;margin-left:auto;margin-right:auto

The boundary IDs in `boundary` correspond to the boundary IDs in the NekRS mesh,
i.e. in the `.re2` file. You can set multiple boundaries to construct by passing a vector of sideset IDs;
this will add multiple boundaries into the same mirror mesh.

!listing test/tests/postprocessors/nek_weighted_side_integral/nek.i
  block=Mesh

!alert warning
When a boundary mesh mirror is constructed using `NekRSMesh`, the actual sidesets
associated with the original mesh are lost. So in the above, for instance, the resulting
mirror mesh would consist of the elements on boundaries 1 through 6 in the NekRS mesh,
but those boundary IDs are not preserved in the mirror. This is a limitation of libMesh,
because *sideset* names cannot be associated with 2-D elements. However, this has not yet
been a limitation in Cardinal, because we can still perform operations based on NekRS
sideset IDs through the various Nek mesh in Cardinal.
If these postprocessors do not meet your needs, you can retain nekRs sideset IDs when
constructing a volume-and-boundary mesh; please see [#vb] for details.

### Volume Mesh Mirror

For applications with volume-based coupling of NekRS to MOOSE, and/or for the more
general case of projecting a NekRS solution on the entire NekRS *volume* to
another MOOSE application, `NekRSMesh` should be used in *volume* mode. This will
read the nodes associated with the entire NekRS mesh and then build an equivalent
`MooseMesh` of either first or second order.

For setting up a volume mesh mirror, we simply need to set `volume = true`.
For example, the following code snippet
would generate a volume mirror of the NekRS mesh shown in [mesh1]. This mesh is
shown in [mesh1_mirrorc].

!listing
[Mesh]
  type = NekRSMesh
  order = FIRST
  volume = true
[]

!media mesh1_mirrorc.png
  id=mesh1_mirrorc
  caption=First-order volume mesh mirror of the NekRS mesh shown in [mesh1]
  style=width:60%;margin-left:auto;margin-right:auto

If we change `order = FIRST` to `order = SECOND`, a second-order volume mesh
will be generated; this second-order mirror of the NekRS mesh in [mesh1] is
shown in [mesh1_mirrord].

!media mesh1_mirrord.png
  id=mesh1_mirrord
  caption=Second-order volume mesh mirror of the NekRS mesh shown in [mesh1]
  style=width:60%;margin-left:auto;margin-right:auto

As a reminder, if creating a mesh mirror in volume mode, there are no sideset
IDs in the resulting mirror mesh.

### Volume and Boundary Mesh Mirrors
  id=vb

Finally, the boundary and volume modes for constructing mesh mirrors can be
combined by specifying both `volume = true` and providing a set of sideset IDs
in `boundary`. The resulting mirror mesh will be a volume mesh, but will also
have sidesets associated with each boundary specified in `boundary`. For instance,
the following code snippet will generate a second-order volume mesh, and boundary
`2` will have a sideset ID in the `MooseMesh`. This mesh is shown in
[mesh1_mirrore], and is the same as shown in [mesh1_mirrord] except that you can
now refer to sideset `2` from within a Cardinal input file or by other MOOSE
applications.

!listing
[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  boundary = '2'
[]

!media mesh1_mirrore.png
  id=mesh1_mirrore
  caption=Second-order volume mesh mirror of the NekRS mesh shown in [mesh1], with boundary `2` assigned an ID
  style=width:60%;margin-left:auto;margin-right:auto

Applications for which you specifically need this combined volume and boundary feature
occur when your coupled MOOSE application wants to treat feedback to NekRS's boundaries
differently depending on their ID in addition to some form of volume-based coupling.
For instance, the combined volume and boundary mesh mirror feature is commonly used
when NekRS is coupled via [!ac](CHT) to MOOSE (where a boundary ID is very helpful in speeding
up the data transfer, by restricting the search only to nodes on the [!ac](CHT) boundary,
as opposed to the entire voluem mesh) *and* is coupled via fluid temperature and density
feedback to a neutronics application (thus necessitating the volume NekRS solution to
be communciated to another MOOSE application).

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

!syntax parameters /Mesh/NekRSMesh

!syntax inputs /Mesh/NekRSMesh

!syntax children /Mesh/NekRSMesh
