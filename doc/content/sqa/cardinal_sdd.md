!template load file=sqa/app_sdd.md.template app=Cardinal category=cardinal

!template! item key=introduction
Many of the phenomena related to nuclear energy systems depend on the solutions
to multiple physics models, which can be described by partial differential equations
that provide spatially- and temporally-varying values of solution variables. When
these models for individual physics depend on one another, we call this "multiphysics."
{{app}} relies on the [OpenMC Monte Carlo radiation transport code](https://docs.openmc.org/en/stable/) and the [NekRS spectral element Navier-Stokes code](https://nekrsdoc.readthedocs.io/en/latest/index.html) to perform high-fidelity multiphysics and
multiscale simulation. {{app}} handles the couplings that may occur between these two
codes, in addition to the numerous physics models provided by the MOOSE framework
(such as solid mechanics, material science, and radiative heat transfer).
This document describes the system design of {{app}}.
!template-end!

!template! item key=system-scope
!include cardinal_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
{{app}} inherits the software dependencies of:
 - [MOOSE framework](framework_sdd.md#dependencies-and-limitations)
 - [OpenMC](https://docs.openmc.org/en/stable/usersguide/install.html#prerequisites)
 - [NekRS](https://github.com/Nek5000/nekRS/tree/next)
 - [DAGMC](https://svalinn.github.io/DAGMC/install/dependencies.html)

No additional dependencies are present.

!template-end!

!template! item key=design-stakeholders
Stakeholders for {{app}} include several of the funding sources including [!ac](DOE), [!ac](INL), and [!ac](ANL).
However, since {{app}} is an open-source project, several universities, companies, and foreign
governments have an interest in the development and maintenance of the {{app}} project.
!template-end!

!template! item key=system-design

{{app}} relies on OpenMC to solve for neutron-photon transport (optionally on
[!ac](CAD) geometries by relying on DAGMC). {{app}} also relies on NekRS to solve
for fluid flow, heat transfer, and species transport. {{app}} integrates these
external libraries within the structure of a MOOSE application, allowing these
external libraries to be coupled to existing capabilities and interfaces in the MOOSE
framework, such as for physics modeling (e.g., with the [MOOSE modules](Modules/index.md) or with other MOOSE applications)
or data processing. Because {{app}} is based upon MOOSE, it employs the same concept
of modular code objects that define all aspects of the solutions for physics.
{{app}} provides specialized

 - [AuxKernels](syntax/AuxKernels/index.md) classes that extract internal solution fields in NekRS and OpenMC
 - [Controls](syntax/Controls/index.md) classes that modify NekRS and OpenMC simulations on-the-fly
 - [Mesh](syntax/Mesh/index.md) classes to build the NekRS and OpenMC geometries in a MOOSE-compatible format, and modify existing meshes useful for postprocessing NekRS and OpenMC simulations
 - [Postprocessors](syntax/Postprocessors/index.md) classes that query the NekRS and OpenMC simulations at points, through spatial integrals, etc.
 - [Problem](syntax/Problem/index.md) classes to execute NekRS and OpenMC as MOOSE applications and facilitate data transfer
 - [TimeStepper](TimeStepper/index.md) classes to control time stepping based on NekRS's adaptive time stepping routines
 - [UserObjects](syntax/UserObjects/index.md) classes that spatially process the NekRS and OpenMC simulations

{{app}} also provides custom syntax for creating OpenMC tallies needed for multiphysics.
!template-end!

!template! item key=system-structure
{{app}} relies on the MOOSE framework to provide the core functionality of solving multiphysics problems. {{app}} replaces the actual physics solves with external [!ac](API) calls to OpenMC and NekRS, but relies on MOOSE for timestepping, synchronization, data transfers, and overall parallelization. Additional physics needed beyond OpenMC and NekRS are obtained from the [MOOSE modules](Modules/index.md).

A summary listing of the current modules required for complete {{app}} operation are shown below:

- [Reactor](reactor/index.md)

{{app}}'s Makefile also includes other modules which are by default enabled, due to
their common usage with Cardinal (though they are not strictly required).

The structure of {{app}} is based on defining C++ classes that derive from classes in the MOOSE
framework or modules that provide functionality that is specifically tailored to nuclear
modeling and simulation. By using the interfaces defined in MOOSE base classes for these classes,
{{app}} is able to rely on MOOSE to execute these models at the appropriate times during the
simulation and use their results in the desired ways.
!template-end!

!syntax complete subsystems=False actions=False objects=False
