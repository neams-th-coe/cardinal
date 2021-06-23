# Cardinal

Cardinal is a wrapping of the GPU-based spectral element CFD code
[nekRS](https://github.com/Nek5000/nekRS) and the Monte Carlo particle
transport code [OpenMC](https://github.com/openmc-dev/openmc) within the
MOOSE framework. Cardinal is intended for providing high-resolution
thermal-hydraulics and/or nuclear heating feedback to MOOSE multiphysics
simulations. [full_pbr] shows an image of a full-core (350,000 pebbles)
nekRS simulation of a [!ac](PBR); multiphysics feedback from calculations
such as these can be incorporated into the MOOSE framework, enabling
a broad set of multiphysics capabilities relevant to nuclear applications.

!media full_pbr.png
  id=full_pbr
  caption=Full-core [!ac](PBR) simulations with nekRS [!cite](lan); MOOSE multiphysics feedback can be incorporated with nekRS and OpenMC simulations with Cardinal
  style=width:80%;margin-left:auto;margin-right:auto

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

Cardinal is design with flexibility in mind. You can use Cardinal to couple
OpenMC and nekRS to the MOOSE framework, as well as run OpenMC and nekRS as standalone
applications - the `openmc` and `nrsmpi` standalone executables are built as part of
the Cardinal build process. Further, you don't need to build any of Cardinal's
dependencies yourself - one Makefile is used to build Cardinal and standalone
executables for OpenMC and nekRS.

!alert note
Cardinal does not support a Nek5000 backend - you must use Cardinal with nekRS.
nekRS is under rapid development, and capabilities continue to grow every day.
At present, the major limitations in nekRS are a lack of wall functions and
the only [!ac](RANS) model being the $k$-$\tau$ model. If your work requires features
in Nek5000 that are not yet available in nekRS, please [contact](https://github.com/Nek5000/nekRS/discussions)
 the nekRS development team. If your needs will not be met in the near future
in nekRS, please contact the Cardinal development team to discuss the possibilities
of adding a Nek5000 backend in Cardinal.

!bibtex bibliography
