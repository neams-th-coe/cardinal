# SkinnedBins

!syntax description /AuxKernels/SkinnedBins

## Description

Displays how each element in the `[Mesh]` maps to a skinned bin distribution
(created by a [MoabSkinner](/userobjects/MoabSkinner.md)). You can either
view the entire individualized bin distribution with `skin_by = all` (the default),
or inspect how elements map to the binning in temperature, density, and subdomain
by setting `skin_by` to `temperature`, `density`, or `subdomain`, respectively.

For example, suppose the domain consists of two subdomains, with temperature and density
distributions as shown in the top row of [moab_skinner2]. The elements are then grouped into
individualized bins, according to unique combinations of the temperature bin, the density
bin, and the subdomain bin. The different `skin_by` options for this object are shown in the
bottom row of [moab_skinner2].

!media moab_skinner2.png
  id=moab_skinner2
  caption=Mesh skinning operation
  style=width:90%;margin-left:auto;margin-right:auto

## Example Input Syntax

As an example, the syntax below separately visualizes how elements map into bins
for all different choices of `skin_by`.

!listing test/tests/userobjects/moab_skinner/all_bins.i
  block=AuxKernels

!syntax parameters /AuxKernels/SkinnedBins

!syntax inputs /AuxKernels/SkinnedBins

!syntax children /AuxKernels/SkinnedBins
