# Multi-Group Cross Section Generation

In this tutorial, you will learn how to:

- Generate [!ac](MGXS) with distributed cell tallies to couple Cardinal with deterministic transport codes
- Modify the resolution of cell-based [!ac](MGXS) by changing the approach used to define the underlying OpenMC geometry
- Use mesh tallies to generate mesh-based cross sections suitable for coarse-mesh diffusion solvers

To access this tutorial,

```
cd cardinal/tutorials/lwr_mgxs
```

!alert! note title=Previous Experience
In this tutorial we assume that the user is familiar with mesh generation with the
[Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html) and running
[!ac](LWR) calculations with Cardinal-OpenMC. The [LWR AMR tutorial](openmc_amr.md) is
used as a base for this tutorial; we recommend it for users which are not familiar with
these concepts.
!alert-end!

## Geometry and Computational Models id=model_1

!include c5g7_ce.md

When generating [!ac](MGXS) tallies do not need to be normalized to power, but Cardinal
still requires a value of reactor power when performing k-eigenvalue calculations. For this tutorial we
select an arbitrary power of 1 Wth, however if you want to compute cross sections which take into account
multi-physics feedback a true reactor power will be required (alongside a coupled thermal-hydraulics solve).

### OpenMC Model id=openmc

The OpenMC model follows standard model building practices for [!ac](LWR) geometries. The Python script used
to generate the `model.xml` file can be found below.

!listing /tutorials/lwr_amr/make_openmc_model.py

!media assembly_amr_openmc.png
  id=assembly_amr_openmc
  caption=OpenMC geometry colored by material ID shown on the $x$-$y$ and $x$-$z$ planes
  style=width:90%;margin-left:auto;margin-right:auto

To generate the XML files needed to run OpenMC, you can run the following:

```bash
python make_openmc_model.py
```

Or you can use the `model.xml` file that is included in the `tutorials/lwr_mgxs` directory.

### Mesh Mirror for MGXS Generation id=mesh

### Neutronics Input File id=neutronics

### Execution and Postprocessing id=exec

## Changing the MGXS Homogenization Volumes id=model_2

### Execution and Postprocessing id=exec_2

## Mesh-Based Cross Sections for Neutron Diffusion Models id=model_3

### Execution and Postprocessing id=exec_3
