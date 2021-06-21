# Tutorials

This page provides several tutorials that cover all major use cases of Cardinal.
By design, Cardinal treats its nekRS and OpenMC wrappings similar to native
MOOSE applications - nekRS and OpenMC can be combined with other MOOSE applications
to provide multiphysics feedback either together or individually. In addition,
nekRS and OpenMC can be run as standalone applications using Cardinal, if you would
like to leverage Cardinal's postprocessing features. There are no limitations on
how nekRS and OpenMC can interact with the MOOSE framework - as long as you can
construct a MultiApp heirarchy that addresses your physics of interest, the existing
[Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) in the MOOSE
framework are all that are needed to send data to/from nekRS and OpenMC and MOOSE.

The following tutorials provide an in-exhaustive description of how to use Cardinal
for a variety of different applications. Because the nekRS and OpenMC wrappings can
interact with *any* MOOSE application, most examples here are shown using the MOOSE
heat conduction module simply for illustration.

This project is under significant active
development; near-term upcoming features are indicated where appropriate.

!alert note
These tutorials assume some familiarity with MOOSE, nekRS, and OpenMC as individual
tools. Please consult the [MOOSE documentation](https://mooseframework.inl.gov/),
[nekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/index.html), and
[OpenMC documentation](https://docs.openmc.org/en/stable/) for code-specific
documentation.

1. [Conjugate Heat Transfer (CHT) coupling of nekRS and MOOSE heat conduction](tutorials/cht.md)
2. [Solid temperature coupling of OpenMC and MOOSE heat conduction](tutorials/openmc_solid.md)
3. [Fluid density and temperature coupling of OpenMC and MOOSE Navier-Stokes](tutorials/openmc_fluid.md)
4. [Multiphysics coupling of nekRS, OpenMC, and MOOSE heat conduction](tutorials/coupled.md)
5. [Multiscale coupling of nekRS and SAM for primary loop modeling](tutorials/sam_coupling.md)
6. [Extracting nekRS turbulent viscosity as an auxiliary variable](tutorials/nekrs_outputs.md)
7. [Running nekRS as a standalone application](tutorials/nekrs_standalone.md)
8. [Running OpenMC as a standalone application](tutorials/openmc_standalone.md)
