# Cardinal

Cardinal is a wrapping of the GPU-based spectral element [!ac](CFD) code
[NekRS](https://github.com/Nek5000/NekRS) and the Monte Carlo particle
transport code [OpenMC](https://github.com/openmc-dev/openmc) within the
MOOSE framework. Cardinal is intended for providing high-resolution
thermal-hydraulics and/or nuclear heating feedback to MOOSE multiphysics
simulations. [full_pbr] shows an image of a full-core (350,000 pebbles)
NekRS simulation of a [!ac](PBR); Cardinal has been demonstrated for
tightly-coupled multiphysics simulations of a 127,000 pebble core
with OpenMC, NekRS, and the MOOSE heat conduction module.
Cardinal allows
multiphysics feedback from calculations
such as these to be incorporated into the MOOSE framework, enabling
a broad set of multiphysics capabilities relevant to nuclear applications.

!media full_pbr.png
  id=full_pbr
  caption=Full-core [!ac](PBR) simulations with NekRS [!cite](lan); MOOSE multiphysics feedback can be incorporated with NekRS and OpenMC simulations via Cardinal
  style=width:80%;margin-left:auto;margin-right:auto

Cardinal allows NekRS and OpenMC to couple seamlessly with the MOOSE framework,
with a number of advantageous features:

- In-memory coupling, eliminating the need to write code-specific I/O programs
  and reducing potential file-based communication bottlenecks
- Distributed parallel meshes for very large-scale problems
- Automatic construction of geometric mappings between the NekRS [!ac](CFD)
  mesh, the OpenMC geometry, and the coupled MOOSE meshes, with virutally no
  requirements on node/element/cell alignment. This eliminates the need for
  rigid one-to-one mappings and enables multiphysics calculations on
  arbitrary geometries.
- Straightforward multiphysics problem setup

Cardinal has
capabilities for [!ac](CHT) coupling of NekRS and MOOSE, concurrent
[!ac](CHT) and volumetric heat source coupling of NekRS and MOOSE,
and volumetric density, temperature, and heat source coupling of OpenMC to MOOSE.
Together, the OpenMC and NekRS wrappings augment MOOSE
by expanding the framework to include high-resolution spectral element [!ac](CFD) and Monte Carlo
particle transport. For [!ac](CHT) applications in particular,
the libMesh-based interpolations of fields between meshes enables fluid-solid
heat transfer simulations on meshes that are not necessarily continuous on phase
interfaces, allowing highly refined fluid boundary layers without also incurring
refined transition layers in the solid regions.

Cardinal is design with flexibility in mind. You can use Cardinal to couple
OpenMC and NekRS to the MOOSE framework, as well as run OpenMC and NekRS as standalone
applications - the `openmc` and `nrsmpi` standalone executables are built as part of
the Cardinal build process. Further, you don't need to build any of Cardinal's
dependencies yourself - one Makefile is used to build Cardinal and standalone
executables for OpenMC and NekRS.

To get started using Cardinal, please check out our [Getting Started](start.md)
page, followed by our [Tutorials](tutorials/index.md).

!alert note
Cardinal does not support a Nek5000 backend - you must use Cardinal with NekRS.
NekRS is under rapid development, and capabilities continue to grow every day.
At present, the major limitations in NekRS are a lack of wall functions and
the only [!ac](RANS) model being the $k$-$\tau$ model. If your work requires features
in Nek5000 that are not yet available in NekRS, please [contact](https://github.com/Nek5000/NekRS/discussions)
 the NekRS development team. If your needs will not be met in the near future
in NekRS, please contact the Cardinal development team to discuss the possibilities
of adding a Nek5000 backend in Cardinal.

!bibtex bibliography
