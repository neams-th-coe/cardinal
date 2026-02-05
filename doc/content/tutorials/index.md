# Tutorials

Cardinal treats its NekRS and OpenMC wrappings similar to native
MOOSE applications - NekRS and OpenMC can be combined with *any* other MOOSE application
to provide multiphysics feedback (either together or individually). In addition,
NekRS and OpenMC can be run as standalone applications using Cardinal, if you would
like to leverage Cardinal's postprocessing and/or Exodus mesh projection features.
There are no limitations on
how NekRS and OpenMC can interact with the MOOSE framework - as long as you can
construct a MultiApp hierarchy that addresses your physics of interest, the existing
[Transfers](Transfers/index.md) in the MOOSE
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
  style=width:70%;margin-left:auto;margin-right:auto;halign:center

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

- [NekRSMesh](NekRSMesh.md)
- [NekTimeStepper](NekTimeStepper.md)
- [NekRSProblem](NekRSProblem.md)
!alert-end!

1. [Introduction to NekRS](nek_intro.md)
2. [Running as a standalone application](nekrs_standalone.md)
3. [Conjugate heat transfer coupling](cht.md)
4. [Forward uncertainty quantification](nekrs_stochastic.md)
5. [Postprocessing/extracting the NekRS solution](nekrs_outputs.md)
6. [Turbulence modeling with NekRS](nek_turbulence.md)

## Tutorials for OpenMC
  id=openmc

!alert! tip
A video recording of the 2nd IAEA/ICTP Workshop, held in Trieste in September 2025,
can be found [here](https://indico.ictp.it/event/10868/other-view?view=ictptimetable).
This workshop provides 3 hours for introduction to OpenMC, 3 hours for introduction
to MOOSE, and 3 hours for an introduction to the OpenMC wrapping in Cardinal.

!alert! tip
These tutorials make use of the following major Cardinal classes. We recommend
quickly reading this documentation before proceeding.

- [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)

Think you're ready for the tutorials? Take our [OpenMC Cardinal quiz](https://www.flexiquiz.com/SC/N/30fc79f0-f9a5-4cfc-b140-c07dda3bdf0b)!
!alert-end!

7. [Temperature, density, and tally coupling to MOOSE](openmc_solid.md)
8. [Adaptive mesh refinement on mesh tallies](openmc_amr.md)
9. [Multi-group cross section generation](openmc_mgxs.md)

## Tutorials for Coupling NekRS and OpenMC
  id=multi

10. [Coupling of NekRS, OpenMC, and MOOSE](coupled.md)

## Miscellaneous

11. [Converting CSG to CAD for multiphysics](csg_to_cad.md)
12. [Restarting coupled NekRS and MOOSE simulations](restart_nek_moose.md)
13. [Coupling to arbitrary MOOSE applications](other_apps.md)
14. [Assessing convergence](convergence.md)
15. [What Transfer should I use?](transfers.md)
