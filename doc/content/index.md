!config navigation breadcrumbs=False scrollspy=False

# Cardinal class=center style=font-weight:200;font-size:400%

!style halign=center fontsize=120%
An open-source coupling of NekRS and OpenMC to MOOSE

!col! small=12 medium=4 large=4 icon=flash_on

### Flexible Multiphysics class=center style=font-weight:200

!style halign=center
Geometry-agnostic RANS and LES conjugate heat transfer,
volumetric source term and multiscale closure coupling, and Monte Carlo nuclear
heating feedback with cell and unstructured mesh tallies
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
Leverages MOOSE user object and postprocessing systems to gain improved
insight into NekRS and OpenMC solutions and provide multiscale closures
to other MOOSE applications
!col-end!

Cardinal is a wrapping of the GPU-oriented spectral element [!ac](CFD) code
[NekRS](https://github.com/Nek5000/NekRS) and the Monte Carlo particle
transport code [OpenMC](https://github.com/openmc-dev/openmc) within the
MOOSE framework. Cardinal provides high-resolution
thermal-hydraulics and/or nuclear heating feedback to MOOSE multiphysics
simulations. Multiphysics feedback is implemented in a geometry-agnostic manner
with virtually no requirements on node/element/cell alignment, eliminating
the need for rigid one-to-one mappings. A generic data transfer implementation
also allows NekRS and OpenMC to be coupled to *any* MOOSE application, enabling
a broad set of multiphysics capabilities. Simulations can also leverage combinations
of MPI, OpenMP, and GPU resources.

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

!media full_pbr.png
  id=full_pbr
  caption=Full-core [!ac](PBR) simulations with NekRS [!cite](lan); MOOSE multiphysics feedback can be incorporated with NekRS and OpenMC simulations via Cardinal and have been applied to fully-coupled OpenMC-NekRS-MOOSE simulations with up to 127,000 pebbles
  style=width:70%;margin-left:auto;margin-right:auto

!media pbr_1568.png
  id=pbr_1568
  caption=Tightly-coupled NekRS-OpenMC-MOOSE simulations of a 1,568 pebble bed [!cite](cardinal2021NT)
  style=width:70%;margin-left:auto;margin-right:auto

!media sfr.png
  id=sfr
  caption=Tightly-coupled NekRS-OpenMC-MOOSE 7-pin [!ac](SFR) bundle simulations [!cite](novak2022)
  style=width:70%;margin-left:auto;margin-right:auto

!bibtex bibliography
