# Tutorials

This page provides several tutorials that cover all major use cases of Cardinal.
By design, Cardinal treats its NekRS and OpenMC wrappings similar to native
MOOSE applications - NekRS and OpenMC can be combined with *any* other MOOSE application
to provide multiphysics feedback either together or individually. In addition,
NekRS and OpenMC can be run as standalone applications using Cardinal, if you would
like to leverage Cardinal's postprocessing and/or Exodus mesh projection features.
There are no limitations on
how NekRS and OpenMC can interact with the MOOSE framework - as long as you can
construct a MultiApp heirarchy that addresses your physics of interest, the existing
[Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) in the MOOSE
framework are all that are needed to send data to/from NekRS and OpenMC and MOOSE.

The following tutorials provide an in-exhaustive description of how to use Cardinal
for a variety of different applications.
Note that because this project is under significant active
development; near-term upcoming features are indicated where appropriate.

Because Cardinal
contains wrappings of both NekRS and OpenMC, you may find that you only want to review
a subset of these tutorials. Within each category, we do recommend moving sequentially;
but with the exception of the tutorials listed in [#multi], the OpenMC tutorials
do not have the NekRS tutorials as pre-requisites, and vice versa.

!alert! note
These tutorials assume some familiarity with MOOSE, NekRS, and OpenMC as individual
tools. Please consult the [MOOSE documentation](https://mooseframework.inl.gov/),
[NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/index.html), and
[OpenMC documentation](https://docs.openmc.org/en/stable/) for code-specific
documentation.
!alert-end!

## Tutorials for NekRS Wrapping

1. [Running as a standalone application](tutorials/nekrs_standalone.md)
2. [Conjugate heat transfer coupling to MOOSE](tutorials/cht.md)
3. [Volumetric heat source coupling to chemical species transport](tutorials/volumetric.md)
4. [Multiscale coupling to SAM for primary loop modeling](tutorials/sam_coupling.md)
5. [Postprocessing/extracting the NekRS solution](tutorials/nekrs_outputs.md)

## Tutorials for OpenMC Wrapping

6. [Solid coupling to MOOSE heat conduction](tutorials/openmc_solid.md)
7. [Solid and fluid coupling of to MOOSE and THM thermal-fluids](tutorials/openmc_fluid.md)

## Tutorials for Combined NekRS and OpenMC Multiphysics
  id=multi

8. [Multiphysics coupling of NekRS, OpenMC, and MOOSE heat conduction](tutorials/coupled.md)

## Miscellaneous

9. [Coupling to arbitrary MOOSE applications](tutorials/other_apps.md)
10. [Assessing convergence](tutorials/convergence.md)
