# September 2025 News

- Per-cell density feedback has been added to both OpenMC and Cardinal:

  - Models requiring detailed density feedback no longer need unique materials per-cell, resulting in an improved quality of life for model preparation and a performance improvement during particle transport;
  - The [MoabSkinner](MoabSkinner.md) no longer adds unique materials for material+density bins.
