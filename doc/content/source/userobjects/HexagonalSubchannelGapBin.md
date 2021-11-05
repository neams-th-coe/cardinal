# HexagonalSubchannelGapBin

!syntax description /UserObjects/HexagonalSubchannelGapBin

## Description

This user object bins the spatial domain according to a unique index
for each gap in a subchannel discretization of an array of pins in
a triangular lattice enclosed by a hexagonal prism. Each gap consists
of the line between two pins or the line between a pin and a duct wall.
For gaps that connect two pins, ordering is done first by sorting on the lower
pin ID in the pair, and next by the higher pin ID in the pair. For instance,
in the gaps shown in [fig1], the interior gaps are ordered as:

- gap 0: pin 0 - pin 1,
- gap 1: pin 0 - pin 2,
- gap 2: pin 0 - pin 3,
- gap 3: pin 0 - pin 4,
- gap 4: pin 0 - pin 5,
- gap 5: pin 0 - pin 6,
- gap 6: pin 1 - pin 2,
- gap 7: pin 1 - pin 6,
- gap 8: pin 1 - pin 7,
- gap 9: pin 1 - pin 8,
- gap 10: pin 1 - pin 18,
- gap 11: pin 2 - pin 3,
- gap 12: pin 2 - pin 8, etc.

For the gaps connecting a pin to a wall, called "peripheral gaps" here, the
ordering is counterclockwise starting with the gap connecting the pin
in the "upper right" corner of the assembly to the top duct wall.

## Example Input Syntax

Below is an example input file that computes gap indices (bins)
for a subchannel discretization.

!listing test/tests/userobjects/hexagonal_gap_bin/subchannel.i
  block=UserObjects

The value of the userobject is shown below; in white are shown the numbering
for the pins (which determine the gap indexing), and in black the numbering for
the gaps.

!media gap_bins.png
  id=fig1
  style=width:60%;margin-left:auto;margin-right:auto
  caption=Subchannel gap bin indices for a 19-pin hexagonal geometry

!syntax parameters /UserObjects/HexagonalSubchannelGapBin

!syntax inputs /UserObjects/HexagonalSubchannelGapBin

!syntax children /UserObjects/HexagonalSubchannelGapBin
