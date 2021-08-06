# KEigenvalue

!syntax description /Postprocessors/KEigenvalue

## Description

This postprocessor extracts the $k$ eigenvalue from the latest OpenMC eigenvalue
calculation. OpenMC contains four different methods for evaluating the $k$ eigenvalue,
each of which can be selected here by setting the `value_type` parameter:

- `collision`: collision estimator
- `absorption`: absorption estimator
- `tracklength`: tracklength estimator
- `combined`: (default) minimum variance estimate based on combining the collision, absorption,
   and tracklength estimates.

## Example Input Syntax

Shown below is an example for each of the four available estimators.

!listing test/tests/postprocessors/eigenvalue/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/KEigenvalue

!syntax inputs /Postprocessors/KEigenvalue

!syntax children /Postprocessors/KEigenvalue
