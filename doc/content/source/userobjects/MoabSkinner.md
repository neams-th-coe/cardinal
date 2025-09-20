# MoabSkinner

!syntax description /UserObjects/MoabSkinner

## Description

This user object bins the spatial domain according to three characteristics:

- temperature (by grouping elements into `n_temperature_bins` with equal spacing between `temperature_min` and `temperature_max`)
- density (by grouping elements into `n_density_bins` with equal spacing between `density_min` and `density_max`)
- subdomain

For example, suppose the domain consists of two subdomains, with temperature and density
distributions as shown in the top row of [moab_skinner]. The elements are then grouped into
individualized bins, according to unique combinations of the temperature bin, the density
bin, and the subdomain bin. In the lower right of [moab_skinner] is then shown the *surface*
skins produced internally, which can be used to define the boundaries of new regions on
which to track particles.

!media moab_skinner.png
  id=moab_skinner
  caption=Mesh skinning operation
  style=width:90%;margin-left:auto;margin-right:auto

!alert tip
You can visualize this user object (i.e., the bin
indices) using a [SpatialUserObjectAux](SpatialUserObjectAux.md).

## Example Input Syntax

Below is an example input file that skins a mesh, generating the bin distributions
shown in [moab_skinner].

!listing test/tests/userobjects/moab_skinner/all_bins.i
  block=UserObjects

The binning, colored on a per-element basis as shown in [moab_skinner] can be visualized
with the [SkinnedBins](SkinnedBins.md) auxiliary kernel.

!syntax parameters /UserObjects/MoabSkinner

!syntax inputs /UserObjects/MoabSkinner
