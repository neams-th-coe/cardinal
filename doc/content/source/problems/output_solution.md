This class enables extracting a number of fields from the NekRS solution
onto the [NekRSMesh](NekRSMesh.md) mesh mirror *in addition* to any
fields that are already interpolated onto the mesh mirror for purposes of
multiphysics coupling to another MOOSE application.
This feature can be used for:

- Quick visualization of the NekRS solution without needing to rely on
  NekRS's custom output format
- Postprocessing the NekRS solution using MOOSE's systems
- Providing one-way coupling to other MOOSE applications, such as for
  transporting scalars based on NekRS's velocity solution or for projecting
  NekRS turbulent viscosity closure terms onto another MOOSE application's mesh

This output feature is used by specifying the fields to be output with the
`output` parameter. Available output fields include:

- `pressure` (which creates a MOOSE variable named `P`)
- `velocity` (which creates MOOSE variables named `vel_x`, `vel_y`, and `vel_z`)
- `temperature` (which creates a MOOSE variable named `temp`)
- `scalar01` (which creates a MOOSE variable named `scalar01`)
- `scalar02` (which creates a MOOSE variable named `scalar02`)
- `scalar03` (which creates a MOOSE variable named `scalar03`)

For NekRS simulations that are coupled to MOOSE, the temperature will already
be output because it is used as part of the physics transfers.

When outputting
data, the NekRS solution fields are interpolated from the [!ac](GLL) points
onto the [NekRSMesh](NekRSMesh.md), which will be either a first or
second order representation of the NekRS mesh. This interpolation is performed
using [Vandermonde matrices](vandermonde.md).
Therefore, the output solution
is generally not an exact representation of NekRS's solution, for which the
NekRS field files are still required to visualize fully. Because the MOOSE
framework supports many different [output formats](Outputs/index.md),
this is a convenient manner by which to obtain a representation of the NekRS
solution in Exodus, VTK, CSV, and other formats.

If `volume = true` is specified on the [NekRSMesh](NekRSMesh.md),
the output solutions are represented over a volume mesh mirror. Otherwise,
if `volume = false`, the solution is shown only on the boundaries specified
with the `boundary` parameter.
