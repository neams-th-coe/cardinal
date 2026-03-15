# CellTally

## Description

The `CellTally` class wraps an OpenMC tally with a distributed cell filter to enable data transfer
from CSG cells to a [MooseMesh](MooseMesh.md) mirror
of the OpenMC geometry. The cell to element mapping established by the [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
is used to facilitate this data transfer to the mesh mirror. When using a `CellTally`, `block`
specifies which blocks in the `[Mesh]` should be tallied. Then, any OpenMC cells that map to those
blocks are added to a cell tally, with one bin for each unique cell ID/instance combination.
`check_equal_mapped_tally_volumes` is used to confirm whether each cell in the `CellTally` has the
same mapped volume on the `MooseMesh` (to a tolerance specified in `equal_tally_volume_abs_tol`).

!include scores_triggers.md

## Random Ray Restrictions

If using OpenMC's [!ac](TRRM) solver, there are
[several restrictions placed on tallies](https://docs.openmc.org/en/stable/usersguide/random_ray.html#tallies).
Namely, filters are limited to [EnergyFilters](EnergyFilter.md) and scores are limited to the following:

- `kappa_fission`
- `flux`
- `total`
- `nu_fission`
- `fission`

In addition to restrictions on filters and scores, global tallies cannot be utilized as they do not apply a
spatial filters; using [!ac](TRRM) requires `normalize_by_global_tally = false` and `check_tally_sum = false`.
Finally, the only `estimator` that can be used with [!ac](TRRM) is the `tracklength` estimator.

## Example Input File Syntax

As an example, this `CellTally` scores `kappa_fission` (the default tally score) on block `0` and stores
the result in a variable named `heat_source`. This corresponds to tallying the heating for three different
fuel pebbles.

!listing /tutorials/pebbles/openmc.i
  block=Problem

!syntax parameters /Problem/Tallies/CellTally

!syntax inputs /Problem/Tallies/CellTally
