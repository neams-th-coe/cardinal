# NekPointValue

!syntax description /Postprocessors/NekPointValue

## Description

This postprocessor interpolates the NekRS solution onto a given
point (using the actual NekRS solution). The provided point does
not need to be a [!ac](GLL) point.

!include /field_specs.md

!include /nondimensional.md

A `function` may also optionally be provided; this function, like all
quantities in Cardinal input files, is given in dimensional form. The
value at the point is then returned as $v-f$, where $v$ is the field
at the point of interest (after being dimensionalized)
and $f$ is the function at that same point.
If the `function` depends on time, that time is either the time of
the start of the next timestep (if using the default of `execute_on = timestep_end`),
or the time of the current timestep (if using `execute_on = timestep_begin`).

## Example Input Syntax

As an example, the following code snippet will interpolate the spectral
NekRS solution onto a provided point.

!listing test/tests/postprocessors/nek_point_value/points.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekPointValue

!syntax inputs /Postprocessors/NekPointValue

!syntax children /Postprocessors/NekPointValue
