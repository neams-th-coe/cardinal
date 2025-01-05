# RadialBin

!syntax description /UserObjects/RadialBin

## Description

This user object bins the spatial domain according to layers in the radial direction.
The vertical direction (about which the radial coordinate is computed) is specified
with the `vertical_axis` parameter. Non-uniform layer sizes can be specified by
setting `growth_r` greater than unity (layers grow in size in increasing radial coordinate)
or `growth_r` less than unity (layers shrink in size in increasing radial coordinate).

For defining bins in an annular radial coordinate system, set `rmin` to the inner
radial coordinate.

## Example Input Syntax

Below is an example input file that constructs layered bins in the radial
direction with uniform, radially increasing, or radially decreasing layer widths.

!listing test/tests/userobjects/radial_bin/radial.i
  block=UserObjects

The bins for radially decreasing layer widths are shown below, colored
by bin ID.

!media radial_bin.png
  id=fig1
  style=width:60%;margin-left:auto;margin-right:auto
  caption=Radial bin indices for shrinking layer widths

!syntax parameters /UserObjects/RadialBin

!syntax inputs /UserObjects/RadialBin
