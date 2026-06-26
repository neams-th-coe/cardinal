# FromXMLFilter

## Description

The `FromXMLFilter` object provides a thin wrapper around an arbitrary OpenMC filter which
has been added to the problem through the `tallies.xml` OpenMC input
file. `FromXMLFilter` fetches the filter through a filter id provided in `filter_id` which
must match the id of a filter in `tallies.xml`. The bins of this filter are labelled with
a string provided in `bin_label`.

!alert warning
`FromXMLFilter` will error if you attempt to use one of OpenMC's functional expansion filters.
These include the [LegendreFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.LegendreFilter.html#openmc.LegendreFilter),
[SpatialLegendreFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.SpatialLegendreFilter.html),
[SphericalHarmonicsFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.SphericalHarmonicsFilter.html),
[ZernikeFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.ZernikeFilter.html),
[ZernikeRadialFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.ZernikeRadialFilter.html),
and [EnergyFunctionFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.EnergyFunctionFilter.html).
Cardinal requires each tally bin in a score to have the same units in order to compute a
tally sum and tally mean for normalization; this may not be the case for tallies which use
a functional expansion filter. This behavior can be overridden by setting `allow_expansion_filters = true`,
however Cardinal may produce erroneous results during tally normalization. It is the user's
responsibility to ensure that any errors caused by the use of a functional expansion
filter are corrected during post-processing or other stages of a Cardinal simulation.

## Example Input File Syntax

As an example, a [MeshTally](MeshTally.md) named `Flux` applies a `FromXMLFilter` named
`SPH_XML` to compute the first four spherical harmonics moments of a `l = 1` spherical
harmonics expansion. The `SphericalHarmonicsFilter` is added through the `tallies.xml`
file with an `id = 1`. Note that `allow_expansion_filters = true`, but no renormalization
is performed so the computed spherical harmonics moments are erroneous.

!listing /tests/neutronics/filters/xml/mesh.i
  block=Problem

!syntax parameters /Problem/Filters/FromXMLFilter

!syntax inputs /Problem/Filters/FromXMLFilter
