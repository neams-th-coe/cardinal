# Adaptive Mesh Refinement on Mesh Tallies

In this tutorial, you will learn how to:

- Couple OpenMC to MOOSE (via a heat source) for a single assembly in the C5G7 [!ac](LWR) reactor physics benchmark
- Generate assembly meshes using the MOOSE [Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html)
- Enable [!ac](AMR) on an unstructured mesh tally to automatically refine the heat source

To access this tutorial,

```
cd cardinal/tutorials/lwr_amr
```

!alert! note title=Computing Needs
This tutorial requires [!ac](HPC) due to the size of the OpenMC model and the number of
bins in the mesh tally, both before and after the application of [!ac](AMR).
!alert-end!

## Geometry and Computational Models

This model consists of a single UO$_2$ assembly from the C5G7 [!ac](LWR) 3D extension case in [!cite](c5g7), where control
rods are fully inserted in the assembly to induce strong axial annd radial gradients. Instead of using the multi-group
cross sections from the C5G7 benchmark specifications, the material properties in [!cite](c5g7_materials) are used.
At a high level the geometry consists of the following lattice elements (in a 17x17 grid):

- 264 fuel pins composed of UO$_2$ pellets clad in zirconium with a helium gap;
- 24 control rods composed of BC$_4$ pellets clad in aluminum (with no gap) which occupy the assembly guide tubes;
- A single fission chamber in the center guide tube composed of borated water with a trace amount of U-235 clad in aluminum (with no gap).

The remainder of the assembly which is not filled with these pincells is composed of water. Above the top of the fuel there is a
reflector region which is penetrated by the control rods. The relevant dimensions can be found in [table1].

!table id=table1 caption=Geometric specifications for a rodded [!ac](LWR) assembly
| Parameter | Value (cm) |
| :- | :- |
| Fuel pellet outer radius | 0.4095 |
| Fuel clad inner radius | 0.418 |
| Fuel clad outer radius | 0.475 |
| Control rod outer radius | 0.3400 |
| Fission chamber outer radius | 0.3400 |
| Guide tube clad outer radius | 0.54 |
| Pin pitch | 1.26 |
| Fuel height | 192.78 |
| Reflector thickness | 21.42 |

A total core power of 3000 MWth is assumed uniformly distributed among 273 fuel assemblies, each with 264 pins
(which do not have a uniform power distribution due to the control rods). The unstructured mesh tallies added by
Cardinal will therefore be normalized according to the per-assembly power:

\begin{equation}
\label{eq:1}
q_{as}=\frac{3000\text{\ MWth}}{n_a}
\end{equation}

where $n_a=273$ is the number of fuel assemblies in the core.

### OpenMC Model

OpenMC's Python [!ac](API) is used to generate the model for this [!ac](LWR) assembly. We begin by defining the
geometric specification of the assembly. This is followed by the creation of materials for each of the regions
described in the previous section, where the helium gap is assumed to be well represented by void. We then define the geometry
for each pincell (fuel, control rod in guide tube, fission chamber, reflector water) which are used to define the 17x17 lattice
for both the active fuel region and the reflector. The sides and bottom of the assembly use reflective boundary conditions while
the top of the assembly uses a vacuum boundary condition. The OpenMC model can be found in [assembly_amr_openmc].

!listing /tutorials/lwr_amr/make_openmc_model.py

!media assembly_amr_openmc.png
  id=assembly_amr_openmc
  caption=OpenMC geometry colored by material ID shown on the $x$-$y$ and $x$-$z$ planes
  style=width:90%;margin-left:auto;margin-right:auto

To generate the XML files needed to run OpenMC, you can run the following:

```
python make_openmc_model.py
```

Or you can use the `model.xml` file that is included in the `tutorials/lwr_amr` directory.

### Mesh Generation

### Neutronics Input File

## Execution and Postprocessing

## Adding Adaptive Mesh Refinement
