# CriticalitySearchHistory

!syntax description /VectorPostprocessors/CriticalitySearchHistory

## Description

This vector postprocessor reports every value tried in the most recent
[CriticalitySearch](AddCriticalitySearchAction.md), in the order tried, as the following vectors:

- `input`: the value of the quantity being searched over, in the units of the search object
- `k`: the mean $k$-eigenvalue
- `k_std_dev`: the standard deviation of the $k$-eigenvalue
- `reactivity`: the reactivity, $\rho=(k-1)/k$, in pcm
- `reactivity_std_dev`: the standard deviation of the reactivity in pcm, from first-order propagation of the uncertainty in $k$

These are the same values which are printed to the console as a table during the search. The vectors
only hold the most recent search; if no search is run on a time step (for instance, because
`criticality_search_on` has been turned off with the MOOSE control system), the vectors retain the values
from the last search which was run.

## Example Input File Syntax

!listing /tests/vectorpostprocessors/criticality_search_history/openmc.i
  block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/CriticalitySearchHistory

!syntax inputs /VectorPostprocessors/CriticalitySearchHistory
