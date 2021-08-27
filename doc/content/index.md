# Cardinal

Cardinal is a wrapping of the GPU-based spectral element [!ac](CFD) code
[NekRS](https://github.com/Nek5000/NekRS) and the Monte Carlo particle
transport code [OpenMC](https://github.com/openmc-dev/openmc) within the
MOOSE framework. Cardinal is intended for providing high-resolution
thermal-hydraulics and/or nuclear heating feedback to MOOSE multiphysics
simulations. Below are shown two select applications in order to
highlight the diversity of high-resolution simulations enabled by Cardinal.

#### Pebble Bed Reactors

[full_pbr] shows an image of a full-core (350,000 pebbles)
NekRS simulation of a [!ac](PBR). Cardinal has been applied to many
[!ac](PBR) geometries, up to 127,000 pebbles with tight coupling of
OpenMC, NekRS, and MOOSE.

!media full_pbr.png
  id=full_pbr
  caption=Full-core [!ac](PBR) simulations with NekRS [!cite](lan); MOOSE multiphysics feedback can be incorporated with NekRS and OpenMC simulations via Cardinal
  style=width:100%;margin-left:auto;margin-right:auto

[pbr_1568] shows a coupled Cardinal simulation of a bed of 1568 pebbles;
on the left is the velocity predicted by NekRS; in the middle is the temperature
predicted by the MOOSE heat conduction module; and on the right is the fission
power predicted by a pebble-wise cell tally in OpenMC.

!media pbr_1568.png
  id=pbr_1568
  caption=1568 pebble bed simulations with NekRS, OpenMC, and the MOOSE heat conduction module [!cite](cardinal2021NT)
  style=width:100%;margin-left:auto;margin-right:auto

#### Sodium Fast Reactors

[sfr] shows a coupled Cardinal calculation of a 7-pin [!ac](SFR) fuel bundle
with tight coupling of OpenMC, NekRS, and MOOSE. On the left is shown
the fission power produced with an unstructured mesh tally in OpenMC. On the right
is shown the fluid temperature (on planes) predicted by NekRS and the duct
temperature (as a volume slice) predicted by the MOOSE heat conduction module.

!media sfr.png
  id=sfr
  caption=7-pin [!ac](SFR) bundle simulations with NekRS, OpenMC, and the MOOSE heat conduction module [!cite](novak2022)
  style=width:100%;margin-left:auto;margin-right:auto

# Capabilities

Cardinal has capabilities for:

- [!ac](CHT) coupling to NekRS
- Volumetric source term coupling to NekRS
- Volumetric source term, density, and temperature coupling to OpenMC

All three of the above features can be combined together for complex
calculations; a geometry-agnostic *and* MOOSE-application-agnostic coupling
allows NekRS and OpenMC to coupled to *any* MOOSE application,
enabling a broad set of multiphysics capabilities. Advantageous features
of Cardinal's design include:

- In-memory coupling, eliminating the need to write code-specific I/O programs
  and reducing potential file-based communication bottlenecks
- Distributed parallel meshes for very large-scale problems
- Automatic construction of geometric mappings between the NekRS [!ac](CFD)
  mesh, the OpenMC geometry, and the coupled MOOSE meshes, with virutally no
  requirements on node/element/cell alignment. This eliminates the need for
  rigid one-to-one mappings and enables multiphysics calculations on
  arbitrary geometries.
- Straightforward multiphysics problem setup

For [!ac](CHT) applications in particular,
the libMesh-based interpolations of fields between meshes enables fluid-solid
heat transfer simulations on meshes that are not necessarily continuous on phase
interfaces, allowing highly refined fluid boundary layers without also incurring
refined transition layers in the solid regions.

To get started using Cardinal, please check out our [Getting Started](start.md)
page, followed by our [Tutorials](tutorials/index.md). Before beginning the tutorials,
you may want to peruse our [source code documentation](source/index.md) for the classes responsible
for wrapping NekRS and OpenMC to review high-level capabilities before diving in
to detailed tutorials.

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
