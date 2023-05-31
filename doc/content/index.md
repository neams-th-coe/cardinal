!config navigation breadcrumbs=False scrollspy=False

# Cardinal class=center style=font-weight:200;font-size:400%

!style halign=center fontsize=120%
Accelerating Discovery in Fusion and Fission Energy

!media assembly_fluid_temp_fine.png
  style=width:100%;margin-left:auto;margin-right:auto

!col! small=12 medium=4 large=4 icon=flash_on

### Flexible Multiphysics class=center style=font-weight:200

!style halign=center
Geometry-agnostic multiphysics with (i) Computational Fluid Dynamics (CFD)
and (ii) Monte Carlo radiation transport on CSG and CAD geometry
!col-end!

!col! small=12 medium=4 large=4 icon=settings

### In-Memory Coupling class=center style=font-weight:200

!style halign=center
In-memory coupling and distributed parallel meshes
to deliver high-resolution multiphysics at scale
and on heterogeneous CPU-GPU systems
!col-end!

!col! small=12 medium=4 large=4 icon=assessment

### Powerful Postprocessing class=center style=font-weight:200

!style halign=center
Leverages MOOSE postprocessing systems for improved data analysis and to
provide multiscale closures
to other MOOSE applications
!col-end!

Cardinal is a wrapping of the GPU-oriented spectral element [!ac](CFD) code
[NekRS](https://github.com/Nek5000/NekRS) and the Monte Carlo particle
transport code [OpenMC](https://github.com/openmc-dev/openmc) within the
MOOSE framework. Cardinal provides high-resolution
thermal-hydraulics and/or radiation transport feedback to MOOSE multiphysics
simulations. Multiphysics feedback is implemented in a geometry-agnostic manner
which eliminates
the need for rigid one-to-one mappings. A generic data transfer implementation
also allows NekRS and OpenMC to couple to *any* MOOSE application, enabling
a broad set of multiphysics capabilities. Simulations can also leverage combinations
of MPI, OpenMP, and GPU resources.

!gallery! large=6
!card media/full_pbr.png title=Pebble Bed Reactors
Full-core PBR simulations with NekRS [!cite](lan); MOOSE finite element heat conduction is coupled with NekRS and OpenMC for up to 127,000 pebbles [!cite](fischer_2021). Figure shows fluid velocity predicted by NekRS.

!card media/cooling_plates.png title=Fusion Components
Multiphysics model of a Helium Cooled Lead Lithium (HCLL) tritium breeder blanket module from the EU DEMO using OpenMC and MOOSE heat conduction [!cite](novak_2023). Image shows the solid temperature in the cooling plates. The OpenMC model uses CAD geometry, capable of highly complex geometry.

!card media/assembly_solid_temp_fine.png title=Prismatic Gas Reactors
Full-core multiphysics model of a high temperature gas reactor using OpenMC-THM-MOOSE [!cite](novak2022_cardinal); figure shows the OpenMC power and MOOSE solid temperature.

!card media/msr_skin.png title=Molten Salt Reactors
Multiphysics model of a Molten Salt Fast Reactor using NekRS-OpenMC [!cite](novak_2023). OpenMC geometry uses CAD, with on-the-fly adaptive re-generation of the OpenMC cells according to contours in temperature and/or density feedback.

!card media/sfr_fluid_planes.png title=Fast Reactors
Multiphysics model of a 7-pin SFR fuel bundle using NekRS-OpenMC-MOOSE [!cite](novak2022); figure shows fluid temperature predicted by a momentum source NekRS model with solid duct temperature predicted by BISON.

!card media/pts.png title=Pressurized Thermal Shock
Coupled NekRS [!ac](CFD) simulations with the MOOSE tensor mechanics module for predicting stress-strain response in [!ac](LWR) reactor vessels [!cite](yu_2022). Figure shows the fluid velocity and temperature predicted by NekRS, which are then fed to a solid mechanics model.

!card media/sam_nek.png title=Systems Analysis
Coupled NekRS [!ac](CFD) simulations with systems-level feedback from SAM, for predicting tracer concentration in a loop with a double T-junction [!cite](huxford2023).
!gallery-end!


!col! small=12 medium=4 large=4 icon=get_app

### Getting Started class=center style=font-weight:200

!style halign=center
Repository is available on [GitHub](https://github.com/neams-th-coe/cardinal) -
learn how to [get started](start.md)
!col-end!

!col! small=12 medium=4 large=4 icon=group

### Tutorials and Documentation class=center style=font-weight:200

!style halign=center
Explore the [tutorials](tutorials/index.md) and [source documentation](source/index.md)
to learn about code capabilities
!col-end!

!col! small=12 medium=4 large=4 icon=assessment

### Build on HPC Systems class=center style=font-weight:200

!style halign=center
Find instructions for building on common
[HPC systems](hpc.md) across the National Laboratories
!col-end!



!bibtex bibliography
