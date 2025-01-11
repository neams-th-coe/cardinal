!template load file=sqa/app_srs.md.template app=Cardinal category=cardinal

!template! item key=system-scope
!! system-scope-begin
Cardinal is an application for performing high-fidelity simulation of nuclear systems
incorporating Monte Carlo neutron-photon transport and/or spectral element [!ac](CFD).
These physics can be combined with one another and with the [MOOSE modules](modules/index.md) to accomplish "multiphysics" simulation. High-fidelity simulations can also be
performed independently, for the purpose of data postprocessing, to generate constitutive
models suitable for lower-fidelity tools, a process referred to as "multiscale" simulation.

Interfaces to other MOOSE-based codes, including systems-level thermal-hydraulics (SAM),
heat pipe flows (Sockeye), and fuel performance (Bison) are also optionally included
to support Cardinal simulations. Cardinal enables high-fidelity modeling of
heat transfer, fluid flow, passive scalar transport, fluid-structure interaction,
nuclear heating, tritium breeding, shielding effectiveness, material activation,
material damage, and sensor response. The [syntax/MultiApps/index.md]
is leveraged to allow for the multiscale, multiphysics coupling. Further, other MOOSE capabilities in the [modules](modules/index.md), such as the [Stochastic Tools Module](stochastic_tools/index.md) enable engineering studies with uncertainty quantification
and sensitivity analysis.
Cardinal therefore supports design, safety, engineering, and research projects.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of Cardinal is to perform fully integrated, high-fidelity, multiphysics simulations of nuclear energy systems at high-fidelity with a
variety of materials, system configurations,
and component designs in order to better understand system performance.
Cardinal's main goal is to bring together the combined multiphysics capabilities of
the [!ac](MOOSE) ecosystem with leading high-performance tools for radiation transport
(OpenMC) and Navier-Stokes fluid flow (NekRS) in an open platform for research,
safety assessment, engineering, and design studies of nuclear energy systems.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
[!ac]({{app}}) has no constraints on hardware and software beyond those of the MOOSE framework, MOOSE modules, OpenMC, NekRS, and DAGMC, as listed in their
respective [!ac](SRS) documents, which are accessible through the links at
the beginning of this document.

[!ac]({{app}}) provides access to a number of code objects that perform computations, such as particle
transport, material behavior, and boundary conditions. These objects each make their own physics-based
assumptions, such as the units of the inputs and outputs. Those assumptions are described in the
documentation for those individual objects.
!template-end!

!template! item key=user-characteristics
{{app}} has two main classes of users:

- +{{app}} Developers+: These are the core developers of {{app}}. They are responsible
  for designing, implementing, and maintaining the software, while following and enforcing its software development standards. These scientists and engineers modify or add capabilities to {{app}}
  for their own purposes, which may include research or extending its capabilities. They will typically
  have a background in thermal-fluids and/or radiation transport, as well as in modeling and simulation techniques.
- +Analysts+: These are users that run {{app}} to run simulations, but do not develop code.
  The primary interface of these users with {{app}} is the input files that define their
  simulations. These users may interact with developers of the system requesting new features and
  reporting bugs found.
!template-end!

!template! item key=information-management
{{app}} as well as the core MOOSE framework, OpenMC, NekRS, and DAGMC are publicly available
on an appropriate repository hosting site. Day-to-day backups and security services will be provided
by the hosting service. More information about backups of the public repository on [!ac](INL)-hosted
services can be found on the following page: [sqa/github_backup.md]
!template-end!

!template! item key=policies-and-regulations
!include framework_srs.md start=policies-and-regulations-begin end=policies-and-regulations-finish
!template-end!

!template! item key=packaging
No special requirements are needed for packaging or shipping any media containing the Cardinal source code. However, some other applications that use {{app}} may be export-controlled, in which case all export control restrictions must be adhered to when
packaging and shipping media.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 90% of all lines of code at all times. Known
regressions will be recorded and tracked (see [#maintainability]) to an independent and
satisfactory resolution.
!template-end!
