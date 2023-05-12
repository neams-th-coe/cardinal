# NekNumRanks

!syntax description /Postprocessors/NekNumRanks

## Description

This postprocessor displays the number of MPI ranks used in the NekRS solve.
This can be useful for understanding how MPI communicators are split up
when running more than one NekRS simulation during a Cardinal run, such as
when you have multiple NekRS physics apps, or are perturbing a case multiple
times through stochastic tools module drivers.

## Example Input Syntax

As an example, the `ranks` postprocessor displays the number of MPI ranks
used to solve NekRS.

!listing test/tests/postprocessors/dimensionless_numbers/dimensional/ranks.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekNumRanks

!syntax inputs /Postprocessors/NekNumRanks

!syntax children /Postprocessors/NekNumRanks
