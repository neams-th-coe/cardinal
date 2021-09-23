# HexagonalSubchannelBin

!syntax description /UserObjects/HexagonalSubchannelBin

## Description

This user object bins the spatial domain according to a unique index
for each channel in a subchannel discretization of an array of pins in
a triangular lattice enclosed by a hexagonal prism. Subchannels are numbered
first for the interior channels, followed by the edge channels, and concluded
with the corner channels. Numbering moves in a counterclockwise direction.

## Example Input Syntax

Below is an example input file that computes channel indices (bins)
for a subchannel discretization.

!listing test/tests/userobjects/hexagonal_subchannel_bin/subchannel.i
  block=UserObjects

The value of the userobject is shown below; numbers superimposed over each
channel are the channel indices for this 19-pin geometry.

!media hex_subchannel_bin.png
  caption=Subchannel bin indices for a 19-pin hexagonal geometry

!syntax parameters /UserObjects/HexagonalSubchannelBin

!syntax inputs /UserObjects/HexagonalSubchannelBin

!syntax children /UserObjects/HexagonalSubchannelBin
