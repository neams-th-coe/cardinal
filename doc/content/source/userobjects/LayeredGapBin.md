# LayeredGapBin

!syntax description /UserObjects/LayeredGapBin

## Description

This user object bins the spatial domain according to a unique index
for planes perpendicular to a specified direction. For instance,
by setting `direction = x`, this user object defines planes perpendicular
to the $x$-axis.

Some user objects allow taking averages of velocity projected onto
the gap planes. The unit normals for the planes are defined to be in the
positive direction.

## Example Input Syntax

Below is an example input file that computes gap indices (bins)
for axial planes perpendicular to the $z$ axis.

!listing test/tests/userobjects/layered_gap_bin/layered_mesh.i
  block=UserObjects

The value of the userobject is shown below; in black is shown the numbering for
the gaps.

!media layer_gap_bins.png
  id=fig1
  style=width:60%;margin-left:auto;margin-right:auto
  caption=Layer gap bin indices for planes perpendicular to the $z$-direction

!syntax parameters /UserObjects/LayeredGapBin

!syntax inputs /UserObjects/LayeredGapBin

!syntax children /UserObjects/LayeredGapBin
