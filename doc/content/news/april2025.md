# April 2025 News

- We have added the ability to [generate multi-group cross sections](SetupMGXSAction.md) in Cardinal:

  - This includes support for distributed cell tallies and unstructured mesh tallies;
  - MGXS results are written to the mesh mirror with the spatial fidelity of the mapped tally objects;
  - Currently supported cross sections / group-wise properties include: total, absorption, nu-scattering, nu-fission, chi, kappa-fission, inverse velocity, and particle diffusion coefficients;
  - Group boundaries can either be supplied, or selected from a list of common group structures
  - Transport corrections are supported for P0 within-group scattering cross sections.
