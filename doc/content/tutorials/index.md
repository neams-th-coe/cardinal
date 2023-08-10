# Tutorials

Cardinal treats its NekRS and OpenMC wrappings similar to native
MOOSE applications - NekRS and OpenMC can be combined with *any* other MOOSE application
to provide multiphysics feedback (either together or individually). In addition,
NekRS and OpenMC can be run as standalone applications using Cardinal, if you would
like to leverage Cardinal's postprocessing and/or Exodus mesh projection features.
There are no limitations on
how NekRS and OpenMC can interact with the MOOSE framework - as long as you can
construct a MultiApp hierarchy that addresses your physics of interest, the existing
[Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) in the MOOSE
framework are all that are needed to send data to/from NekRS and OpenMC and MOOSE.

[cardinal_multiapp] illustrates how NekRS and OpenMC interact with MOOSE -
at a high level, "OpenMC talks to MOOSE" and "NekRS talks to MOOSE," such that
any simulation coupling OpenMC to NekRS is actually just communicating through a
MOOSE "intermediary."
So, if you are interested in multiphysics coupling of OpenMC and NekRS, all you need
to understand is how NekRS and OpenMC *individually* couple to MOOSE, which is
covered by the [#nekrs] and [#openmc] sections
below. Nevertheless, we do also include some tutorials that specifically couple both
NekRS and OpenMC simultaneously, in the [#multi] section.
Within each category, we do recommend moving sequentially.

!media framework.png
  id=cardinal_multiapp
  caption=High-level illustration of how NekRS and OpenMC couple to MOOSE
  style=width:90%;margin-left:auto;margin-right:auto;halign:center

Note that because this project is under significant active
development; near-term upcoming features are indicated where appropriate.

!alert! note title=Prerequisites
These tutorials assume some familiarity with MOOSE, NekRS, and OpenMC as individual
tools. Please consult the [MOOSE documentation](https://mooseframework.inl.gov/),
[NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/index.html), and
[OpenMC documentation](https://docs.openmc.org/en/stable/) for code-specific
documentation.

These tutorials will make use of different classes in Cardinal, all of which are also
documented [here](source/index.md).
!alert-end!

!alert! note title=Computing Needs
Many of these tutorials were developed with NEAMS and NRIC VTB support.
As such, many are "production"-type models developed for other programs, and may use very
fine meshes. We will indicate required computing resources for each tutorial. If you
have access to [!ac](HPC) resources, check out our [HPC guide](hpc.md) to find example
job submission scripts, which you can adapt.
Regardless of your computing capacity, we recommend reading all tutorials, as they contain a lot of useful information
on how to approach multiphysics coupling.
!alert-end!

## Tutorials for NekRS
  id=nekrs

!alert! tip
These tutorials make use of the following major Cardinal classes.
We recommend quickly reading this documentation before proceeding.

- [NekRSMesh](/mesh/NekRSMesh.md)
- [NekTimeStepper](/timesteppers/NekTimeStepper.md)
- [NekRSProblem](/problems/NekRSProblem.md)
- [NekRSStandaloneProblem](/problems/NekRSStandaloneProblem.md)
!alert-end!

1. [Running as a standalone application](tutorials/nekrs_standalone.md)
2. [Conjugate heat transfer coupling to MOOSE](tutorials/cht.md)
3. [Multiscale coupling to SAM for primary loop modeling](tutorials/sam_coupling.md)
4. [Forward uncertainty quantification](tutorials/nekrs_stochastic.md)
5. [Postprocessing/extracting the NekRS solution](tutorials/nekrs_outputs.md)

## Tutorials for OpenMC
  id=openmc

!alert! tip
These tutorials make use of the following major Cardinal classes. We recommend
quickly reading this documentation before proceeding.

- [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)

Think you're ready for the tutorials? Take our [OpenMC Cardinal quiz](https://www.flexiquiz.com/SC/N/30fc79f0-f9a5-4cfc-b140-c07dda3bdf0b)!
!alert-end!

6. [Solid coupling to MOOSE](tutorials/openmc_solid.md)
7. [Solid and fluid coupling to MOOSE](tutorials/openmc_fluid.md)

## Tutorials for Coupling NekRS and OpenMC
  id=multi

8. [Coupling of NekRS, OpenMC, and MOOSE](tutorials/coupled.md)

## Miscellaneous

9. [Restarting coupled NekRS and MOOSE simulations](tutorials/restart_nek_moose.md)
10. [Coupling to arbitrary MOOSE applications](tutorials/other_apps.md)
11. [Assessing convergence for OpenMC calculations](tutorials/convergence.md)
12. [What Transfer should I use?](tutorials/transfers.md)
