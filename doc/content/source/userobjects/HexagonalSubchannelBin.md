# HexagonalSubchannelBin

!syntax description /UserObjects/HexagonalSubchannelBin

## Description

This user object bins the spatial domain according to a unique index
for each channel in a subchannel discretization of an array of pins in
a triangular lattice enclosed by a hexagonal prism. Subchannels are numbered
first for the interior channels, followed by the edge channels, and concluded
with the corner channels. Numbering moves in a counterclockwise direction.

Alternatively, if you set `pin_centered_bins` to true, then the bins are
centered on the pins (as opposed to a channel-based discretization). This will
create one hexagonal prism around each pin, with a single bin area forming any
remaining area of the bundle (i.e. between the outer ring of pins and the duct).
The bins are numbered first by ring and then in a counterclockwise direction.

When using `pin_centered_bins = false`, this user object can be paired with
integrals/averages over subchannels. When using `pin_centered_bins = true`,
this user object can be paired with side integrals/averages to compute quantities
on the pin surfaces.

## Example Input Syntax

Below is an example input file that computes channel indices (bins)
for a subchannel discretization.

!listing test/tests/userobjects/hexagonal_subchannel_bin/subchannel.i
  block=UserObjects

The value of the userobject is shown below; numbers superimposed over each
channel are the channel indices for this 19-pin geometry.

!media hex_subchannel_bin.png
  id=fig1
  style=width:60%;margin-left:auto;margin-right:auto
  caption=Subchannel bin indices for a 19-pin hexagonal geometry

Below is a second example that sets `pin_centered_bins` to true, which
instead defines bins as hexagons centered on the pins.

!listing test/tests/userobjects/hexagonal_subchannel_bin/pin_centered_3.i
  block = UserObjects

The value of the userobject is shown below; numbers superimposed over
each bin are the pin indices (plus one additional index for the peripheral
region) for this 19-pin geometry.

!media hex_subchannel_pin_bin.png
  id=fig2
  style=width:60%;margin-left:auto;margin-right:auto
  caption=Pin-centered bin indices for a 19-pin hexagonal geometry

!syntax parameters /UserObjects/HexagonalSubchannelBin

!syntax inputs /UserObjects/HexagonalSubchannelBin
