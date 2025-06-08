# AddFieldTransferAction

## Overview

The `AddFieldTransferAction` is responsible for performing data transfers between NekRS and MOOSE of field variables (quantities in NekRS defined at the nodal points, distributed in space throughout a mesh). This is done with the `[FieldTransfers]` block in a Cardinal input file.
Examples of field transfers which can be added include:

- [NekBoundaryFlux](NekBoundaryFlux.md), to exchange a flux through a boundary
- [NekVolumetricSource](NekVolumetricSource.md), to exchange a volumetric source term
- [NekMeshDeformation](NekMeshDeformation.md), to exchange a mesh displacement
- [NekFieldVariable](NekFieldVariable.md), to exchange a field variable

## Creating Variables

When writing data `to_nek`, Cardinal will read from an [AuxVariable](AuxVariable.md); when reading
data `from_nek`, Cardinal will write that data into an [AuxVariable](AuxVariable.md). Therefore,
this object also automatically creates the needed AuxVariable using the name of the object.
The order of the auxvariable is matched to the order of the [NekRSMesh](NekRSMesh.md).

## Passing Data

The order of the internal NekRS mesh may not necessarily match that of [NekRSMesh](NekRSMesh.md).
Therefore, this object will interpolate between the NekRS [!ac](GLL) points and the
[NekRSMesh](NekRSMesh.md) using [Vandermonde matrices](vandermonde.md).

## Usrwrk Array

When `direction = to_nek`, data is "sent" into NekRS by writing into the `nrs->usrwrk` scratch space array and also copying from this array into its corresponding array on the host (`nrs->o_usrwrk`)
so that the data is accessible both on host and device within NekRS. For field transfers,
each field occupies a single "slot" in the `nrs->usrwrk` array. For transfers into NekRS,
you are required to list which slot in the array to write. Note that Cardinal automatically
allocates this array, but you can control the size of it by setting `n_usrwrk_slots`
in [NekRSProblem](NekRSProblem.md).

!include seg_fault_warning.md

## Example Input File Syntax

As an example, a [NekBoundaryFlux](NekBoundaryFlux.md) and a [NekVolumetricSource](NekVolumetricSource.md) are added in the `[Problem/FieldTransfers]` block. These objects
automatically create auxvariables with names `avg_flux` and `heat_source`, respectively; these
variables will be read from in order to write data into NekRS.
A [NekFieldVariable](NekFieldVariable.md) is also added, which will create a variable named
`temp`; this auxiliary variable will receive the NekRS internal field variable, temperature.

!listing /tests/conduction/boundary_and_volume/prism/nek.i
  block=Problem

When running this case, Cardinal will print out a table showing a summary of what data is
occupying the various slots in the usrwrk arrays and how they can be accessed. This table
for the above example is listed below, which shows that heat flux values will exist in the
first slice and volumetric heat source values will exist in the second slice.

```
-----------------------------------------------------------------------------------------------------------
| Slot | MOOSE quantity |          How to Access (.oudf)          |         How to Access (.udf)          |
-----------------------------------------------------------------------------------------------------------
|    0 | avg_flux       | bc->usrwrk[0 * bc->fieldOffset+bc->idM] | nrs->usrwrk[0 * nrs->fieldOffset + n] |
|    1 | heat_source    | bc->usrwrk[1 * bc->fieldOffset+bc->idM] | nrs->usrwrk[1 * nrs->fieldOffset + n] |
|    2 | unused         | bc->usrwrk[2 * bc->fieldOffset+bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
|    3 | unused         | bc->usrwrk[3 * bc->fieldOffset+bc->idM] | nrs->usrwrk[3 * nrs->fieldOffset + n] |
|    4 | unused         | bc->usrwrk[4 * bc->fieldOffset+bc->idM] | nrs->usrwrk[4 * nrs->fieldOffset + n] |
|    5 | unused         | bc->usrwrk[5 * bc->fieldOffset+bc->idM] | nrs->usrwrk[5 * nrs->fieldOffset + n] |
|    6 | unused         | bc->usrwrk[6 * bc->fieldOffset+bc->idM] | nrs->usrwrk[6 * nrs->fieldOffset + n] |
-----------------------------------------------------------------------------------------------------------
```

For instance, the heat flux is then used in a boundary condition on the device in the `.oudf` file.

!listing /tests/conduction/boundary_and_volume/prism/pyramid.oudf language=cpp
  re=void\sscalarNeumannConditions.*?^


!syntax list /Problem/FieldTransfers actions=false subsystems=false heading=Available FieldTransfer Objects

!syntax parameters /Problem/FieldTransfers/AddFieldTransferAction
