# August 2026 News

## Generic Field Binning in MoabSkinner

The [MoabSkinner](MoabSkinner.md) user object now supports binning by an arbitrary number of
user-specified fields through the new `fields`, `fields_min`, `fields_max`, and `n_field_bins`
parameters, rather than being restricted to temperature and density. This makes it possible to
skin a mesh according to fields such as burnup, enrichment, or any other auxiliary variable, in
addition to temperature and density.

The deprecated `temperature`, `temperature_min`, `temperature_max`, `n_temperature_bins`,
`density`, `density_min`, `density_max`, and `n_density_bins` parameters have been removed.
Inputs using these parameters will now error; use the `fields` syntax instead.