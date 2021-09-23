# LayeredBin

!syntax description /UserObjects/LayeredBin

## Description

This user object bins the spatial domain according to uniform-size
layers in a specified direction according to the bounding box of the mesh.
Layers are numbered increasing in the positive direction.

## Example Input Syntax

Below is an example input file that constructs layered bins in each
of the three coordinate directions.

!listing test/tests/userobjects/layered_bin/layered.i
  block=UserObjects

The bins in the $x$ direction are shown below, colored by bin ID.

!media layered_bin.png
  caption=Layered bin indices in the $x$ direction

!syntax parameters /UserObjects/LayeredBin

!syntax inputs /UserObjects/LayeredBin

!syntax children /UserObjects/LayeredBin
