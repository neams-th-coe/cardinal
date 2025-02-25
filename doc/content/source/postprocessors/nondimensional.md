If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales with the [Dimensionalize](DimensionalizeAction.md) sub-block
for the `[Problem]`) then the value of this object
is shown in *dimensional* units. If this object supports a `field` parameter, then the only
exception will be for the NekRS scalars
(scalar01, scalar02 or scalar03), which
are never dimensionalized because their dimensions are problem-dependent.
