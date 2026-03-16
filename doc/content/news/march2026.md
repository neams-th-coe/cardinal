# March 2026 News

- We have added first-class multiphysics support for OpenMC's random ray solver to Cardinal. This includes support for temperature feedback, density feedback, and deforming geometry (via DAGMC and the [MoabSkinner](MoabSkinner.md)). Note that temperature feedback requires a multi-group cross section library with multiple temperature datapoints.

  - Temperature and density feedback support has been verified with the [Doppler slab benchmark](doppler_slab.md).

  - Deforming geometry support has been verified (in a preliminary manner) with the [Griesheimer and Kooreman benchmark](s2_thermal_expansion.md).

  - Documentation for the random ray addition can be found [here](OpenMCCellAverageProblem.md#random_ray), and a tutorial for this capability can be found [here](rr_lwr_pincell.md).
