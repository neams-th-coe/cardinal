# NekVolumetricData

## Description

`NekVolumetricData` is a [FieldTransfer](AddFieldTransferAction.md) that sends a general volumetric field between NekRS and MOOSE. For instance, this object can be used to pass an entire volumetric field from the scratch space into NekRS. This object is effectively the same
as [NekVolumetricSource](NekVolumetricSource.md), except that this latter object is
specifically only for heat sources (in which case there is additional normalization applied).
First, this object
creates an [AuxVariable](AuxVariable.md) using the name of the object; this variable will hold
the volumetric quantity which NekRS reads (for `direction = to_nek`) or will be written by NekRS (for
`direction = from_nek`).

## Example Input File Syntax

As an example, the example below fills the scratch space slots with auxiliary variables
named `usrwrk00`, `usrwrk01`, and `usrwrk02` and passes these into the scratch space
inside NekRS.

!listing tests/postprocessors/nek_point_value/points.i
  block=Problem

!syntax parameters /Problem/FieldTransfers/NekVolumetricData

!syntax inputs /Problem/FieldTransfers/NekVolumetricData
