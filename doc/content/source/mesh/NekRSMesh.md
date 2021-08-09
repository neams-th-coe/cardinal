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
data is *written* into NekRS, it is read from a `MooseVariable` on the `NekRSMesh`
and interpolated directly into boundary condition and source arrays in NekRS.

A mesh mirror can be constructed for

- Just the boundaries in a NekRS domain. This is used for cases where only boundary coupling/data is desired, such
  as for [!ac](CHT) coupling.
- The entire NekRS volume. This is used for cases where only volume coupling/data is desired, such
  as for temperature and density coupling to OpenMC, or for extracting the NekRS solution over the volume.
- The entire NekRS volume *and* boundaries. This is used for cases where boundary *and*/or volume
  coupled/data is desired, such as for combined [!ac](CHT) and volume coupling to BISON and OpenMC,
  or for extracting the NekRS solution over the volume.

Each of these three modes are now described.

### Boundary Mesh Mirror

Boundary mesh mirrors are used for:

- [!ac](CHT) coupling of NekRS to MOOSE

In boundary mode, `NekRSMesh` will read the nodes associated with
all NekRS elements on a set of specified boundaries and
then build an equivalent `MooseMesh` of either first or second order. For example, consider
the NekRS mesh shown in [mesh1]; a boundary with ID 2 is indicated. Mesh lines are shown
connecting the [!ac](GLL) quadrature points in NekRS (*not* the eight nodes that would be needed to
define the corners of a hexahedral element).

!media mesh1.png
  id=mesh1
  caption=Example NekRS mesh; mesh lines are shown connecting [!ac](GLL) points
  style=width:60%;margin-left:auto;margin-right:auto

The code snippet below shows how to construct a first-order representation of boundary 2 as a `MooseMesh`.
We set the `order` of the mesh to `FIRST`, meaning that NekRS's elements are converted
to HEX8. That is, if NekRS uses a polynomial order of 7, data transfers between the
mesh mirror and NekRS's mesh would involve interpolations on each element between
$(7+1)^2$ NekRS quadrature points and 4 MOOSE nodes.

!listing /test/tests/conduction/nonidentical_interface/cylinders/nek.i
  block=Mesh

The mesh mirror produced by `NekRSMesh` is shown in [mesh1_mirror]. The four nodes
on each QUAD4 element are shown as white dots.

!media mesh1_mirror.png
  id=mesh1_mirror
  caption=First-order surface mesh mirror of the NekRS mesh shown in [mesh1], for sideset `2`.
  style=width:40%;margin-left:auto;margin-right:auto

By default, a first-order mesh is constructed. But you can also create a second-order
mesh by setting `order = SECOND`. If we change `order = FIRST` in
to `order = SECOND`, the surface elements are now represented as QUAD9 elements,
as shown in [mesh1_mirrorb].
That is, if NekRS uses a polynomial order of 7, data
transfers between the mesh mirror and NekRS's mesh would involve interpolations on each
element between $(7+1)^2$ NekRS quadrature points and 9 MOOSE nodes.

!media mesh1_mirrorb.png
  id=mesh1_mirrorb
  caption=Second-order surface mesh mirror of the NekRS mesh shown in [mesh1], for sideset `2`.
  style=width:40%;margin-left:auto;margin-right:auto

The boundary IDs in `boundary` correspond to the boundary IDs in the NekRS mesh,
i.e. in the `.re2` file.
You can set multiple boundaries to construct by passing a vector of sideset IDs;
this will add multiple boundaries into the same mirror mesh.

The `boundary` parameter does more than just inform which
sides should be created in the mesh mirror - the `boundary` parameter also establishes
a number of internal mapping data structures used to write/read data from the
corresponding NekRS sidesets. That is, if you set `boundary = 1`, you are indicating
that boundary 1 in NekRS's mesh will participate in [!ac](CHT) coupling to MOOSE.

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

Volume mesh mirrors are used for:

- Coupling NekRS via volume temperatures and densities to MOOSE, such as for
  fluid feedback to OpenMC
- Projecting a NekRS volume solution onto a lower-order MOOSE mesh

In volume mode, `NekRSMesh` will
read the nodes associated with the entire NekRS mesh and then build an equivalent
`MooseMesh` of either first or second order.

For setting up a volume mesh mirror, we simply need to set `volume = true`.
For example, the following code snippet
would generate a volume mirror of the NekRS mesh shown in [mesh1]. This mesh is
shown in [mesh1_mirrorc].
That is, if NekRS uses a polynomial order of 7, data
transfers between the mesh mirror and NekRS's mesh would involve interpolations on each
element between $(7+1)^3$ NekRS quadrature points and 8 MOOSE nodes.

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
That is, if NekRS uses a polynomial order of 7, data
transfers between the mesh mirror and NekRS's mesh would involve interpolations on each
element between $(7+1)^3$ NekRS quadrature points and 27 MOOSE nodes.

!media mesh1_mirrord.png
  id=mesh1_mirrord
  caption=Second-order volume mesh mirror of the NekRS mesh shown in [mesh1]
  style=width:60%;margin-left:auto;margin-right:auto

If creating a mesh mirror in volume mode, the sideset IDs from the NekRS mesh
will be constructed in the mesh mirror.
However, volume mode does *not* allow
boundary coupling of NekRS to a coupled MOOSE application because the
`boundary` parameter is not used to specify which boundaries should be *coupled*.
That is, the sidesets will exist in the volume mesh, but no data on
those boundaries are used to send data in/out of NekRS's boundaries.
So, the volume *and* boundary mode should be used if combined [!ac](CHT)
and volume coupling are desired; see [#vb] for details.

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
2 will be coupled via [!ac](CHT) to Moose.

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
