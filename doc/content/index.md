# Cardinal

Cardinal is a wrapping of the GPU-based spectral element CFD code
[nekRS](https://github.com/Nek5000/nekRS) and the Monte Carlo particle
transport code [OpenMC](https://github.com/openmc-dev/openmc) within the
MOOSE framework. Cardinal is intended for providing high-resolution
thermal-hydraulics and/or particle transport feedback to MOOSE multiphysics
simulations.

Cardinal allows nekRS and OpenMC to couple seamlessly with the MOOSE framework,
enabling in-memory coupling, distributed parallel meshes for very large-scale
applications, and straightforward multiphysics problem setup. Cardinal has
capabilities for conjugate heat transfer coupling of nekRS and MOOSE, concurrent
conjugate heat transfer and volumetric heat source coupling of nekRS and MOOSE,
and volumetric density, temperature, and heat source coupling of OpenMC to MOOSE.
Together, the OpenMC and nekRS wrappings augment MOOSE
by expanding the framework to include high-resolution spectral element [!ac](CFD) and Monte Carlo
particle transport. For conjugate heat transfer applications in particular,
the libMesh-based interpolations of fields between meshes enables fluid-solid
heat transfer simulations on meshes that are not necessarily continuous on phase
interfaces, allowing mesh resolution to be specified based on the underlying physics,
rather than rigid continuity restrictions in single-application heat transfer codes.

Cardinal is designed with high performance computing and flexibility in mind.

