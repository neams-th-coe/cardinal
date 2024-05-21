# KRUSTY

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE for the KRUSTY reactor
- Control the resolution of physics feedback received by OpenMC

To access this tutorial,

```
cd cardinal/tutorials/krusty
```

## Geometry and Computational Models

This model consists of a multiphysics simulation of the KRUSTY reactor, one of the simplest space power reactor concepts ever proposed. The relevant dimensions are summarized in [table1] and in [krusty_solid_mesh] shows the geometry and gap thicknesses. The basic components are the fuel, heat pipes, control, reflectors and shield. For the sake of simplification, we will only simulate the components within the region spanning from the bottom axial reflector to the top axial reflector along the z-direction, which entails neglecting components outside of this region, the control rod insertion is also being disregarded.The fuel is high enriched uranium U-7.65 Mo with a total length of 25 cm, an outer diameter of 11 cm, and the core contains a 4 cm hole to allow $B_4C$ control insertion. 

!table id=table1 caption=Geometric specifications for KRUSTY
| Component       | Inner diameter (cm)  | Outer diameter (cm)  |
|-----------------|----------------------|----------------------|
| Fuel            | 4.000                | 11.000               |
| Clamp           | 11.812               | 12.130               |
| Vacuum can      | 12.995               | 13.300               |
| Sleeve          | 14.211               | 14.300               |
| Reflector       | 14.500               | 38.100               |
| Shield          | 41.000               | 101.900              |
| Heat pipe wall  | 1.181                | 1.270                |


Currently, ongoing work is underway to benchmark Cardinal against KRUSTY, where the neglected components will be considered. This will relax the simplifications made in this example.


!media krusty_solid_mesh.png
  id=krusty_solid_mesh
  caption=KRUSTY geometry schematic [!cite](PostonGibsonGodfroy2020).
  style=width:45%

### MOOSE Heat Conduction Model

The MOOSE heat transfer module is used to solve for steady-state heat conduction
\begin{equation}
\label{eq:1}
-\nabla \cdot (k_s \nabla T_s) = \dot{q}_s
\end{equation}
where $k_s$ is the solid thermal conductivity, $T_s$ is the solid temperature, and $\dot{q}_s$  is a volumetric heat source in the solid.

[krusty_mesh] shows the mesh used in this example. The geometry was build using FUSION360, where the CAD file was exported into Cubit, where the mesh was build. Cubit can create meshes with user-defined geometry and customizable boundary layers. The Cubit script used to generate the solid mesh can be found in the directory `cardinal/tutorials/krusty/meshes`, where you will find two files, a `.sat` file that you will import into cubit, which will generate the geometry of KRUSTY, however in order to generete the mesh you will need to run the scripts in the `.txt` file in the cubit terminal, and then you can export the `.e` mesh file, which is the format that MOOSE can read. You can find the `.e` files in this box link: [box](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499). Do not forget to place the `e.` mesh files in the same directory as your `.i` files.

!media krusty_mesh.png
  id=krusty_mesh
  caption=KRUSTY mesh
  style=width:45%

### OpenMC Model

The OpenMC model is built using CSG, which are cells created from regions of space formed by half-space of various common surfaces. First, we define materials for each component of the reactor. Next, we define our geometric parameters, so we can start building our surfaces and manipulate them to create the geometry of each component of the KRUSTY reactor. Then, we need to create cells for each component and fill them with the materials that we previously created, so we can create a universe containg all the cells using `openmc.Universe`. This geometry can be seen in [krusty_solid_mesh]. 

We will use a linear-linear stochastic interpolation between the two cross section data sets nearest to the imposed temperature by setting the `method` parameter on `settings.temperature` to `interpolation`. When OpenMC is initialized, the cross section data is loaded for a temperature range specified by `range` in `settings`. 

!listing /tutorials/krusty/KRUSTY/model03.py

To generate the settings XML files needed to run OpenMC, you just need to run 

```
settings.export_to_xml()
```


## Multiphysics Coupling
  id=coupling

In this section, OpenMC, Sockeye are coupled to MOOSE for heat source and heat pipes temperature feedback for the KRUSTY reactor. First we briefly discuss the basic thermal cycle of a heat pipe, which is described in [fig_heat_pipe]

!media fig_heat_pipe.png
  id=fig_heat_pipe
  caption=KRUSTY geometry schematic [!cite](wikiHP).
  style=width:45%
where $1$ shows the working fluid evaporating to vapour by absorbing thermal energy, at $2$ the vapor migrates along the cavity to lower temperature end, and at $3$ the vapour condenses back to fluid and is absorbed by the wick, releasing thermal energy. Finally, at $4$ the working fluid flows back to the higher temperature end. The following sub-sections will describe the input file for each application.




### Solid Input Files

The solid phase is solved with the MOOSE Heat Transfer module, and is described in the [solide.i](/tutorials/krusty/KRUSTY/solide.i) input. The mesh exodus file was generated in the Cubit script mentioned before. This file is called inside the `[Mesh]` block:

!listing /tutorials/krusty/KRUSTY/solide.i
  end=Variables


The heat transfer module will solve for temperature, which we define as a nonlinear variable and apply a simple uniform initial condition of $1073 K$

!listing /tutorials/krusty/KRUSTY/solide.i
  block=Variables

The governing equation solved by MOOSE is specified in the `[Kernels]` block with the [HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html) and [CoupledForce](https://mooseframework.inl.gov/source/kernels/CoupledForce.html) kernels.

!listing /tutorials/krusty/KRUSTY/solide.i
  block=Kernels
  
Next, the boundary conditions on the solid are applied. In case A, coupling only OpenMC and MOOSE Heat Transfer Model, the boundary conditions on each heat pipe will be the Dirichlet type, where they have a constant temperature of $1073 K$ and for the "outside" boundary conditions will be a [ConvectiveHeatFluxBC](https://mooseframework.inl.gov/source/bcs/ConvectiveHeatFluxBC.html) type, where it computes the convective heat flux, with the convective heat transfer coefficient and the far field temperature coupled as material properties.

!listing /tutorials/krusty/KRUSTY/solide.i
  block=BCs

The [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html) system in MOOSE is used to communicate variables across applications; a heat source will be computed by OpenMC and applied as a source term in MOOSE. In the opposite direction, MOOSE will compute a temperature that will be applied to the OpenMC geometry. We need to add an auxiliary variable to receive the heat source from OpenMC, which is defined as `[power]`.


!listing /tutorials/krusty/KRUSTY/solide.i
  block=AuxVariables

We also set thermal conductivity values in the blocks: fuel, `cavity_center`, `gap_clamp`, `gap_ref`, `gap_sleeve`, `gap_vaccan`. Constant values are used for simplicity.

!listing /tutorials/krusty/KRUSTY/solide.i
  block=Materials


By setting a fixed number of time steps, `dt`, this example will simply run a fixed number of Picard iterations.

!listing /tutorials/krusty/KRUSTY/solide.i
  block=Executioner

MOOSE will provide the solid temperature for OpenMC so we add postprocessors to evaluate the average, minimum and maximum temperature in the fuel. 

!listing /tutorials/krusty/KRUSTY/solide.i
  start=Postprocessors

### Neutronics Input Files

We begin by defining a mesh on which OpenMC will receive temperature from the couple MOOSE application.

!listing /tutorials/krusty/KRUSTY/openmc.i
  end=AuxVariables

Then, we can define our initial conditions for the temperature, which will be a constant function of $1073 K$

!listing /tutorials/krusty/KRUSTY/openmc.i
  start=ICs
  end=Problem

Next, we define the `[Problem]` block, which will describe all objects necessary for the physics solve. In order to replace MOOSE finite element calculations with OpenMC particle transport calculations, we are going to use the [OpenMCCellAverageProblem](https://cardinal.cels.anl.gov/source/problems/OpenMCCellAverageProblem.html) class. In this model, we specify the total fission power by which to normalize OpenMC's tally results. Next, we indicate which blocks in the `[Mesh]` should be considered for temperature feedback using the `temperature_blocks` parameter. Mesh tallies were used to estimate the heat source, as indicated by `tally_type`, where the same mesh can be translated to multiple locations in the OpenMC geometry, specifying a mesh file using `mesh_template`.

!listing /tutorials/krusty/KRUSTY/openmc.i
  block=Problem
  
Next, we add a series of auxiliary variables for solution visualization 

!listing /tutorials/krusty/KRUSTY/openmc.i
  block=AuxVariables

In this example, the overall calculation workflow is as follows:

1. Run OpenMC using the initial condition for the temperature.
2. Run MOOSE heat conduction with a given power distribuition from OpenMC.
3. Run OpenMC with an updated temperature distribution.
4. Extract the kappa-fission distribution (the recoverable fission energy)
  computed by OpenMC and map in the opposite direction from OpenMC cells
  to all the MOOSE elements that mapped to each cell.

The above sequence is repeated until desired convergence of the coupled domain is achieved.
The [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks describe the interaction between Cardinal and MOOSE. OpenMC is run as the main application, with
MOOSE heat conduction run as the sub-application.


!listing /tutorials/krusty/KRUSTY/openmc.i
  start=MultiApps
  end=Postprocessors

For the heat source transfer from OpenMC, we ensure conservation by requiring that the integral of heat source computed by OpenMC matches the integral of the heat source received by MOOSE. We also add a postprocessor to evaluate the Tally relative error and the multiplication factor.

!listing /tutorials/krusty/KRUSTY/openmc.i
  block=Postprocessors

Because we did specify sub-cycling in the `[MultiApps]` block, we do not need to define a number of time steps here.

!listing /tutorials/krusty/KRUSTY/openmc.i
  start=Executioner


## Execution and Postprocessing

To run the coupled calculation

```
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

This will run both MOOSE and OpenMC with 2 MPI processes and 2 OpenMP threads per rank.
To run the simulation faster, you can increase the parallel processes/threads, or
simply decrease the number of particles used in OpenMC.
When the simulation has completed, you will have created a number of different output files:

- `openmc_out_solid0.e`, an Exodus output with the OpenMC solution and the data
  that was ultimately transferred in/out of OpenMC

First, let's examine how the mapping between OpenMC and MOOSE was established.
When we run with `verbose = true`, you will see the following mapping information displayed:

```
 ===================>     MAPPING FROM OPENMC TO MOOSE     <===================

          T:      # elems providing temperature-only feedback
          rho:    # elems providing density-only feedback
          T+rho:  # elems providing temperature and density feedback
          Other:  # elems which do not provide feedback to OpenMC
                    (but receives a cell tally from OpenMC)
     Mapped Vol:  volume of MOOSE elems each cell maps to
     Actual Vol:  OpenMC cell volume (computed with 'volume_calculation')

----------------------------------------------------------------------------------------
|            Cell            |   T   |  rho  | T+rho | Other | Mapped Vol | Actual Vol |
----------------------------------------------------------------------------------------
|   1, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.272e-06  |            |
|   2, instance   0 (of   1) |    27 |     0 |     0 |     0 | 2.093e-06  |            |
|   3, instance   0 (of   1) |    37 |     0 |     0 |     0 | 2.994e-06  |            |
|   4, instance   0 (of   1) |    43 |     0 |     0 |     0 | 3.343e-06  |            |
|   5, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.264e-06  |            |
|   6, instance   0 (of   1) |    40 |     0 |     0 |     0 | 2.794e-06  |            |
|   7, instance   0 (of   1) |    55 |     0 |     0 |     0 | 3.334e-06  |            |
|   8, instance   0 (of   1) |    55 |     0 |     0 |     0 | 2.914e-06  |            |
|   9, instance   0 (of   1) |    63 |     0 |     0 |     0 | 3.281e-06  |            |
|  10, instance   0 (of   1) |    54 |     0 |     0 |     0 | 2.142e-06  |            |
|  11, instance   0 (of   1) |   116 |     0 |     0 |     0 | 4.344e-06  |            |
|  12, instance   0 (of   1) |   103 |     0 |     0 |     0 | 3.510e-06  |            |
|  13, instance   0 (of   1) |   117 |     0 |     0 |     0 | 3.284e-06  |            |
|  14, instance   0 (of   1) |   159 |     0 |     0 |     0 | 3.858e-06  |            |
|  15, instance   0 (of   1) |   223 |     0 |     0 |     0 | 4.373e-06  |            |
|  16, instance   0 (of   1) |   219 |     0 |     0 |     0 | 3.930e-06  |            |
|  17, instance   0 (of   1) |   328 |     0 |     0 |     0 | 3.638e-06  |            |
|  18, instance   0 (of   1) |   378 |     0 |     0 |     0 | 4.023e-06  |            |
|  19, instance   0 (of   1) |   255 |     0 |     0 |     0 | 3.619e-06  |            |
|  20, instance   0 (of   1) |   264 |     0 |     0 |     0 | 3.659e-06  |            |
|  21, instance   0 (of   1) |   164 |     0 |     0 |     0 | 2.034e-06  |            |
|  22, instance   0 (of   1) |   222 |     0 |     0 |     0 | 4.005e-06  |            |
|  23, instance   0 (of   1) |   234 |     0 |     0 |     0 | 4.125e-06  |            |
|  24, instance   0 (of   1) |   274 |     0 |     0 |     0 | 3.551e-06  |            |
|  25, instance   0 (of   1) |    29 |     0 |     0 |     0 | 2.053e-06  |            |
|  26, instance   0 (of   1) |    16 |     0 |     0 |     0 | 1.200e-06  |            |
|  27, instance   0 (of   1) |    50 |     0 |     0 |     0 | 3.710e-06  |            |
|  28, instance   0 (of   1) |    51 |     0 |     0 |     0 | 3.533e-06  |            |
|  29, instance   0 (of   1) |     7 |     0 |     0 |     0 | 4.522e-07  |            |
|  30, instance   0 (of   1) |    49 |     0 |     0 |     0 | 2.709e-06  |            |
|  31, instance   0 (of   1) |    69 |     0 |     0 |     0 | 3.249e-06  |            |
|  32, instance   0 (of   1) |   104 |     0 |     0 |     0 | 3.975e-06  |            |
|  33, instance   0 (of   1) |    87 |     0 |     0 |     0 | 2.775e-06  |            |
|  34, instance   0 (of   1) |   125 |     0 |     0 |     0 | 3.186e-06  |            |
|  35, instance   0 (of   1) |   145 |     0 |     0 |     0 | 3.363e-06  |            |
|  36, instance   0 (of   1) |   160 |     0 |     0 |     0 | 3.265e-06  |            |
|  37, instance   0 (of   1) |   207 |     0 |     0 |     0 | 3.640e-06  |            |
|  38, instance   0 (of   1) |   234 |     0 |     0 |     0 | 3.712e-06  |            |
|  39, instance   0 (of   1) |   308 |     0 |     0 |     0 | 4.054e-06  |            |
|  40, instance   0 (of   1) |   297 |     0 |     0 |     0 | 3.800e-06  |            |
|  41, instance   0 (of   1) |   494 |     0 |     0 |     0 | 4.716e-06  |            |
|  42, instance   0 (of   1) |   438 |     0 |     0 |     0 | 3.904e-06  |            |
|  43, instance   0 (of   1) |   286 |     0 |     0 |     0 | 3.770e-06  |            |
|  44, instance   0 (of   1) |   302 |     0 |     0 |     0 | 3.712e-06  |            |
|  45, instance   0 (of   1) |   192 |     0 |     0 |     0 | 2.109e-06  |            |
|  46, instance   0 (of   1) |   232 |     0 |     0 |     0 | 3.156e-06  |            |
|  47, instance   0 (of   1) |   300 |     0 |     0 |     0 | 4.063e-06  |            |
|  48, instance   0 (of   1) |   320 |     0 |     0 |     0 | 3.652e-06  |            |
|  49, instance   0 (of   1) |    25 |     0 |     0 |     0 | 1.776e-06  |            |
|  50, instance   0 (of   1) |    21 |     0 |     0 |     0 | 1.567e-06  |            |
|  51, instance   0 (of   1) |    41 |     0 |     0 |     0 | 2.972e-06  |            |
|  52, instance   0 (of   1) |    56 |     0 |     0 |     0 | 3.821e-06  |            |
|  53, instance   0 (of   1) |    10 |     0 |     0 |     0 | 6.322e-07  |            |
|  54, instance   0 (of   1) |    58 |     0 |     0 |     0 | 3.127e-06  |            |
|  55, instance   0 (of   1) |    70 |     0 |     0 |     0 | 3.040e-06  |            |
|  56, instance   0 (of   1) |    94 |     0 |     0 |     0 | 3.442e-06  |            |
|  57, instance   0 (of   1) |    79 |     0 |     0 |     0 | 2.288e-06  |            |
|  58, instance   0 (of   1) |   134 |     0 |     0 |     0 | 3.309e-06  |            |
|  59, instance   0 (of   1) |   160 |     0 |     0 |     0 | 3.543e-06  |            |
|  60, instance   0 (of   1) |   176 |     0 |     0 |     0 | 3.534e-06  |            |
|  61, instance   0 (of   1) |   204 |     0 |     0 |     0 | 3.589e-06  |            |
|  62, instance   0 (of   1) |   243 |     0 |     0 |     0 | 3.828e-06  |            |
|  63, instance   0 (of   1) |   303 |     0 |     0 |     0 | 3.813e-06  |            |
|  64, instance   0 (of   1) |   309 |     0 |     0 |     0 | 3.565e-06  |            |
|  65, instance   0 (of   1) |   549 |     0 |     0 |     0 | 5.176e-06  |            |
|  66, instance   0 (of   1) |   474 |     0 |     0 |     0 | 3.719e-06  |            |
|  67, instance   0 (of   1) |   296 |     0 |     0 |     0 | 3.345e-06  |            |
|  68, instance   0 (of   1) |   341 |     0 |     0 |     0 | 4.042e-06  |            |
|  69, instance   0 (of   1) |   202 |     0 |     0 |     0 | 1.983e-06  |            |
|  70, instance   0 (of   1) |   238 |     0 |     0 |     0 | 3.434e-06  |            |
|  71, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.969e-06  |            |
|  72, instance   0 (of   1) |   321 |     0 |     0 |     0 | 3.803e-06  |            |
|  73, instance   0 (of   1) |    24 |     0 |     0 |     0 | 1.714e-06  |            |
|  74, instance   0 (of   1) |    16 |     0 |     0 |     0 | 1.197e-06  |            |
|  75, instance   0 (of   1) |    49 |     0 |     0 |     0 | 3.625e-06  |            |
|  76, instance   0 (of   1) |    58 |     0 |     0 |     0 | 4.232e-06  |            |
|  77, instance   0 (of   1) |     8 |     0 |     0 |     0 | 4.345e-07  |            |
|  78, instance   0 (of   1) |    53 |     0 |     0 |     0 | 3.001e-06  |            |
|  79, instance   0 (of   1) |    71 |     0 |     0 |     0 | 3.474e-06  |            |
|  80, instance   0 (of   1) |    92 |     0 |     0 |     0 | 3.424e-06  |            |
|  81, instance   0 (of   1) |    71 |     0 |     0 |     0 | 2.378e-06  |            |
|  82, instance   0 (of   1) |   124 |     0 |     0 |     0 | 3.199e-06  |            |
|  83, instance   0 (of   1) |   160 |     0 |     0 |     0 | 3.575e-06  |            |
|  84, instance   0 (of   1) |   163 |     0 |     0 |     0 | 3.190e-06  |            |
|  85, instance   0 (of   1) |   200 |     0 |     0 |     0 | 3.521e-06  |            |
|  86, instance   0 (of   1) |   252 |     0 |     0 |     0 | 4.046e-06  |            |
|  87, instance   0 (of   1) |   309 |     0 |     0 |     0 | 3.913e-06  |            |
|  88, instance   0 (of   1) |   309 |     0 |     0 |     0 | 3.586e-06  |            |
|  89, instance   0 (of   1) |   560 |     0 |     0 |     0 | 5.075e-06  |            |
|  90, instance   0 (of   1) |   449 |     0 |     0 |     0 | 3.332e-06  |            |
|  91, instance   0 (of   1) |   334 |     0 |     0 |     0 | 3.625e-06  |            |
|  92, instance   0 (of   1) |   359 |     0 |     0 |     0 | 4.137e-06  |            |
|  93, instance   0 (of   1) |   187 |     0 |     0 |     0 | 1.605e-06  |            |
|  94, instance   0 (of   1) |   263 |     0 |     0 |     0 | 3.669e-06  |            |
|  95, instance   0 (of   1) |   281 |     0 |     0 |     0 | 3.929e-06  |            |
|  96, instance   0 (of   1) |   305 |     0 |     0 |     0 | 3.767e-06  |            |
|  97, instance   0 (of   1) |    21 |     0 |     0 |     0 | 1.595e-06  |            |
|  98, instance   0 (of   1) |    26 |     0 |     0 |     0 | 2.072e-06  |            |
|  99, instance   0 (of   1) |    45 |     0 |     0 |     0 | 3.322e-06  |            |
| 100, instance   0 (of   1) |    58 |     0 |     0 |     0 | 3.805e-06  |            |
| 101, instance   0 (of   1) |     5 |     0 |     0 |     0 | 2.318e-07  |            |
| 102, instance   0 (of   1) |    47 |     0 |     0 |     0 | 2.437e-06  |            |
| 103, instance   0 (of   1) |    71 |     0 |     0 |     0 | 3.122e-06  |            |
| 104, instance   0 (of   1) |   104 |     0 |     0 |     0 | 3.830e-06  |            |
| 105, instance   0 (of   1) |    74 |     0 |     0 |     0 | 2.452e-06  |            |
| 106, instance   0 (of   1) |   118 |     0 |     0 |     0 | 3.221e-06  |            |
| 107, instance   0 (of   1) |   153 |     0 |     0 |     0 | 3.495e-06  |            |
| 108, instance   0 (of   1) |   198 |     0 |     0 |     0 | 4.092e-06  |            |
| 109, instance   0 (of   1) |   163 |     0 |     0 |     0 | 3.037e-06  |            |
| 110, instance   0 (of   1) |   242 |     0 |     0 |     0 | 4.023e-06  |            |
| 111, instance   0 (of   1) |   296 |     0 |     0 |     0 | 3.849e-06  |            |
| 112, instance   0 (of   1) |   302 |     0 |     0 |     0 | 3.663e-06  |            |
| 113, instance   0 (of   1) |   553 |     0 |     0 |     0 | 5.083e-06  |            |
| 114, instance   0 (of   1) |   506 |     0 |     0 |     0 | 3.884e-06  |            |
| 115, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.164e-06  |            |
| 116, instance   0 (of   1) |   375 |     0 |     0 |     0 | 4.346e-06  |            |
| 117, instance   0 (of   1) |   213 |     0 |     0 |     0 | 1.910e-06  |            |
| 118, instance   0 (of   1) |   246 |     0 |     0 |     0 | 3.334e-06  |            |
| 119, instance   0 (of   1) |   288 |     0 |     0 |     0 | 3.996e-06  |            |
| 120, instance   0 (of   1) |   330 |     0 |     0 |     0 | 3.929e-06  |            |
| 121, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.232e-06  |            |
| 122, instance   0 (of   1) |    19 |     0 |     0 |     0 | 1.500e-06  |            |
| 123, instance   0 (of   1) |    50 |     0 |     0 |     0 | 3.566e-06  |            |
| 124, instance   0 (of   1) |    65 |     0 |     0 |     0 | 4.379e-06  |            |
| 125, instance   0 (of   1) |     2 |     0 |     0 |     0 | 1.224e-07  |            |
| 126, instance   0 (of   1) |    62 |     0 |     0 |     0 | 3.197e-06  |            |
| 127, instance   0 (of   1) |    72 |     0 |     0 |     0 | 3.289e-06  |            |
| 128, instance   0 (of   1) |   103 |     0 |     0 |     0 | 3.834e-06  |            |
| 129, instance   0 (of   1) |    68 |     0 |     0 |     0 | 2.192e-06  |            |
| 130, instance   0 (of   1) |   105 |     0 |     0 |     0 | 2.671e-06  |            |
| 131, instance   0 (of   1) |   164 |     0 |     0 |     0 | 3.820e-06  |            |
| 132, instance   0 (of   1) |   183 |     0 |     0 |     0 | 3.752e-06  |            |
| 133, instance   0 (of   1) |   195 |     0 |     0 |     0 | 3.496e-06  |            |
| 134, instance   0 (of   1) |   248 |     0 |     0 |     0 | 3.695e-06  |            |
| 135, instance   0 (of   1) |   310 |     0 |     0 |     0 | 4.022e-06  |            |
| 136, instance   0 (of   1) |   306 |     0 |     0 |     0 | 3.626e-06  |            |
| 137, instance   0 (of   1) |   518 |     0 |     0 |     0 | 4.869e-06  |            |
| 138, instance   0 (of   1) |   463 |     0 |     0 |     0 | 3.724e-06  |            |
| 139, instance   0 (of   1) |   293 |     0 |     0 |     0 | 3.157e-06  |            |
| 140, instance   0 (of   1) |   357 |     0 |     0 |     0 | 4.015e-06  |            |
| 141, instance   0 (of   1) |   216 |     0 |     0 |     0 | 2.169e-06  |            |
| 142, instance   0 (of   1) |   252 |     0 |     0 |     0 | 3.477e-06  |            |
| 143, instance   0 (of   1) |   291 |     0 |     0 |     0 | 3.936e-06  |            |
| 144, instance   0 (of   1) |   312 |     0 |     0 |     0 | 3.653e-06  |            |
| 145, instance   0 (of   1) |    16 |     0 |     0 |     0 | 1.124e-06  |            |
| 146, instance   0 (of   1) |    26 |     0 |     0 |     0 | 1.862e-06  |            |
| 147, instance   0 (of   1) |    45 |     0 |     0 |     0 | 3.125e-06  |            |
| 148, instance   0 (of   1) |    59 |     0 |     0 |     0 | 3.730e-06  |            |
| 149, instance   0 (of   1) |     9 |     0 |     0 |     0 | 5.350e-07  |            |
| 150, instance   0 (of   1) |    55 |     0 |     0 |     0 | 2.778e-06  |            |
| 151, instance   0 (of   1) |    74 |     0 |     0 |     0 | 2.972e-06  |            |
| 152, instance   0 (of   1) |   114 |     0 |     0 |     0 | 3.901e-06  |            |
| 153, instance   0 (of   1) |    69 |     0 |     0 |     0 | 2.107e-06  |            |
| 154, instance   0 (of   1) |   145 |     0 |     0 |     0 | 3.591e-06  |            |
| 155, instance   0 (of   1) |   163 |     0 |     0 |     0 | 3.624e-06  |            |
| 156, instance   0 (of   1) |   178 |     0 |     0 |     0 | 3.581e-06  |            |
| 157, instance   0 (of   1) |   193 |     0 |     0 |     0 | 3.476e-06  |            |
| 158, instance   0 (of   1) |   244 |     0 |     0 |     0 | 3.990e-06  |            |
| 159, instance   0 (of   1) |   310 |     0 |     0 |     0 | 3.974e-06  |            |
| 160, instance   0 (of   1) |   301 |     0 |     0 |     0 | 3.393e-06  |            |
| 161, instance   0 (of   1) |   547 |     0 |     0 |     0 | 5.109e-06  |            |
| 162, instance   0 (of   1) |   498 |     0 |     0 |     0 | 3.829e-06  |            |
| 163, instance   0 (of   1) |   304 |     0 |     0 |     0 | 3.137e-06  |            |
| 164, instance   0 (of   1) |   371 |     0 |     0 |     0 | 4.219e-06  |            |
| 165, instance   0 (of   1) |   211 |     0 |     0 |     0 | 2.064e-06  |            |
| 166, instance   0 (of   1) |   241 |     0 |     0 |     0 | 3.342e-06  |            |
| 167, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.884e-06  |            |
| 168, instance   0 (of   1) |   326 |     0 |     0 |     0 | 3.842e-06  |            |
| 169, instance   0 (of   1) |    18 |     0 |     0 |     0 | 1.264e-06  |            |
| 170, instance   0 (of   1) |    33 |     0 |     0 |     0 | 2.567e-06  |            |
| 171, instance   0 (of   1) |    49 |     0 |     0 |     0 | 3.408e-06  |            |
| 172, instance   0 (of   1) |    69 |     0 |     0 |     0 | 4.007e-06  |            |
| 173, instance   0 (of   1) |    13 |     0 |     0 |     0 | 6.894e-07  |            |
| 174, instance   0 (of   1) |    58 |     0 |     0 |     0 | 2.807e-06  |            |
| 175, instance   0 (of   1) |    80 |     0 |     0 |     0 | 3.109e-06  |            |
| 176, instance   0 (of   1) |   119 |     0 |     0 |     0 | 3.921e-06  |            |
| 177, instance   0 (of   1) |    56 |     0 |     0 |     0 | 1.624e-06  |            |
| 178, instance   0 (of   1) |   146 |     0 |     0 |     0 | 3.484e-06  |            |
| 179, instance   0 (of   1) |   167 |     0 |     0 |     0 | 3.719e-06  |            |
| 180, instance   0 (of   1) |   193 |     0 |     0 |     0 | 3.694e-06  |            |
| 181, instance   0 (of   1) |   182 |     0 |     0 |     0 | 2.965e-06  |            |
| 182, instance   0 (of   1) |   268 |     0 |     0 |     0 | 4.048e-06  |            |
| 183, instance   0 (of   1) |   320 |     0 |     0 |     0 | 3.923e-06  |            |
| 184, instance   0 (of   1) |   295 |     0 |     0 |     0 | 3.280e-06  |            |
| 185, instance   0 (of   1) |   575 |     0 |     0 |     0 | 5.212e-06  |            |
| 186, instance   0 (of   1) |   503 |     0 |     0 |     0 | 3.939e-06  |            |
| 187, instance   0 (of   1) |   321 |     0 |     0 |     0 | 3.211e-06  |            |
| 188, instance   0 (of   1) |   368 |     0 |     0 |     0 | 4.163e-06  |            |
| 189, instance   0 (of   1) |   221 |     0 |     0 |     0 | 2.117e-06  |            |
| 190, instance   0 (of   1) |   265 |     0 |     0 |     0 | 3.485e-06  |            |
| 191, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.851e-06  |            |
| 192, instance   0 (of   1) |   345 |     0 |     0 |     0 | 4.029e-06  |            |
| 193, instance   0 (of   1) |     8 |     0 |     0 |     0 | 5.505e-07  |            |
| 194, instance   0 (of   1) |    27 |     0 |     0 |     0 | 2.067e-06  |            |
| 195, instance   0 (of   1) |    51 |     0 |     0 |     0 | 3.401e-06  |            |
| 196, instance   0 (of   1) |    55 |     0 |     0 |     0 | 3.247e-06  |            |
| 197, instance   0 (of   1) |    24 |     0 |     0 |     0 | 1.171e-06  |            |
| 198, instance   0 (of   1) |    55 |     0 |     0 |     0 | 2.409e-06  |            |
| 199, instance   0 (of   1) |    91 |     0 |     0 |     0 | 3.283e-06  |            |
| 200, instance   0 (of   1) |   126 |     0 |     0 |     0 | 4.087e-06  |            |
| 201, instance   0 (of   1) |    82 |     0 |     0 |     0 | 2.172e-06  |            |
| 202, instance   0 (of   1) |   134 |     0 |     0 |     0 | 3.118e-06  |            |
| 203, instance   0 (of   1) |   173 |     0 |     0 |     0 | 3.759e-06  |            |
| 204, instance   0 (of   1) |   203 |     0 |     0 |     0 | 3.950e-06  |            |
| 205, instance   0 (of   1) |   194 |     0 |     0 |     0 | 3.290e-06  |            |
| 206, instance   0 (of   1) |   257 |     0 |     0 |     0 | 3.859e-06  |            |
| 207, instance   0 (of   1) |   327 |     0 |     0 |     0 | 4.011e-06  |            |
| 208, instance   0 (of   1) |   303 |     0 |     0 |     0 | 3.386e-06  |            |
| 209, instance   0 (of   1) |   535 |     0 |     0 |     0 | 4.827e-06  |            |
| 210, instance   0 (of   1) |   499 |     0 |     0 |     0 | 4.032e-06  |            |
| 211, instance   0 (of   1) |   301 |     0 |     0 |     0 | 3.035e-06  |            |
| 212, instance   0 (of   1) |   359 |     0 |     0 |     0 | 4.062e-06  |            |
| 213, instance   0 (of   1) |   225 |     0 |     0 |     0 | 2.272e-06  |            |
| 214, instance   0 (of   1) |   249 |     0 |     0 |     0 | 3.327e-06  |            |
| 215, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.966e-06  |            |
| 216, instance   0 (of   1) |   322 |     0 |     0 |     0 | 3.657e-06  |            |
| 217, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.188e-06  |            |
| 218, instance   0 (of   1) |    32 |     0 |     0 |     0 | 2.460e-06  |            |
| 219, instance   0 (of   1) |    51 |     0 |     0 |     0 | 3.485e-06  |            |
| 220, instance   0 (of   1) |    61 |     0 |     0 |     0 | 3.527e-06  |            |
| 221, instance   0 (of   1) |    25 |     0 |     0 |     0 | 1.310e-06  |            |
| 222, instance   0 (of   1) |    63 |     0 |     0 |     0 | 2.759e-06  |            |
| 223, instance   0 (of   1) |    81 |     0 |     0 |     0 | 3.104e-06  |            |
| 224, instance   0 (of   1) |   119 |     0 |     0 |     0 | 3.749e-06  |            |
| 225, instance   0 (of   1) |    79 |     0 |     0 |     0 | 2.102e-06  |            |
| 226, instance   0 (of   1) |   131 |     0 |     0 |     0 | 2.929e-06  |            |
| 227, instance   0 (of   1) |   169 |     0 |     0 |     0 | 3.535e-06  |            |
| 228, instance   0 (of   1) |   202 |     0 |     0 |     0 | 3.768e-06  |            |
| 229, instance   0 (of   1) |   202 |     0 |     0 |     0 | 3.318e-06  |            |
| 230, instance   0 (of   1) |   248 |     0 |     0 |     0 | 3.671e-06  |            |
| 231, instance   0 (of   1) |   336 |     0 |     0 |     0 | 4.194e-06  |            |
| 232, instance   0 (of   1) |   313 |     0 |     0 |     0 | 3.635e-06  |            |
| 233, instance   0 (of   1) |   536 |     0 |     0 |     0 | 4.955e-06  |            |
| 234, instance   0 (of   1) |   489 |     0 |     0 |     0 | 3.843e-06  |            |
| 235, instance   0 (of   1) |   304 |     0 |     0 |     0 | 3.198e-06  |            |
| 236, instance   0 (of   1) |   374 |     0 |     0 |     0 | 4.275e-06  |            |
| 237, instance   0 (of   1) |   200 |     0 |     0 |     0 | 1.746e-06  |            |
| 238, instance   0 (of   1) |   263 |     0 |     0 |     0 | 3.544e-06  |            |
| 239, instance   0 (of   1) |   300 |     0 |     0 |     0 | 4.012e-06  |            |
| 240, instance   0 (of   1) |   342 |     0 |     0 |     0 | 3.925e-06  |            |
| 241, instance   0 (of   1) |     8 |     0 |     0 |     0 | 5.457e-07  |            |
| 242, instance   0 (of   1) |    30 |     0 |     0 |     0 | 2.239e-06  |            |
| 243, instance   0 (of   1) |    54 |     0 |     0 |     0 | 3.673e-06  |            |
| 244, instance   0 (of   1) |    65 |     0 |     0 |     0 | 4.073e-06  |            |
| 245, instance   0 (of   1) |    10 |     0 |     0 |     0 | 4.650e-07  |            |
| 246, instance   0 (of   1) |    56 |     0 |     0 |     0 | 2.619e-06  |            |
| 247, instance   0 (of   1) |    86 |     0 |     0 |     0 | 3.274e-06  |            |
| 248, instance   0 (of   1) |   121 |     0 |     0 |     0 | 3.866e-06  |            |
| 249, instance   0 (of   1) |    78 |     0 |     0 |     0 | 2.147e-06  |            |
| 250, instance   0 (of   1) |   146 |     0 |     0 |     0 | 3.446e-06  |            |
| 251, instance   0 (of   1) |   171 |     0 |     0 |     0 | 3.627e-06  |            |
| 252, instance   0 (of   1) |   186 |     0 |     0 |     0 | 3.561e-06  |            |
| 253, instance   0 (of   1) |   204 |     0 |     0 |     0 | 3.477e-06  |            |
| 254, instance   0 (of   1) |   257 |     0 |     0 |     0 | 3.922e-06  |            |
| 255, instance   0 (of   1) |   303 |     0 |     0 |     0 | 3.795e-06  |            |
| 256, instance   0 (of   1) |   313 |     0 |     0 |     0 | 3.531e-06  |            |
| 257, instance   0 (of   1) |   555 |     0 |     0 |     0 | 5.108e-06  |            |
| 258, instance   0 (of   1) |   462 |     0 |     0 |     0 | 3.446e-06  |            |
| 259, instance   0 (of   1) |   304 |     0 |     0 |     0 | 3.315e-06  |            |
| 260, instance   0 (of   1) |   377 |     0 |     0 |     0 | 4.311e-06  |            |
| 261, instance   0 (of   1) |   198 |     0 |     0 |     0 | 1.816e-06  |            |
| 262, instance   0 (of   1) |   256 |     0 |     0 |     0 | 3.556e-06  |            |
| 263, instance   0 (of   1) |   301 |     0 |     0 |     0 | 3.842e-06  |            |
| 264, instance   0 (of   1) |   321 |     0 |     0 |     0 | 3.825e-06  |            |
| 265, instance   0 (of   1) |    15 |     0 |     0 |     0 | 1.063e-06  |            |
| 266, instance   0 (of   1) |    24 |     0 |     0 |     0 | 1.870e-06  |            |
| 267, instance   0 (of   1) |    48 |     0 |     0 |     0 | 3.361e-06  |            |
| 268, instance   0 (of   1) |    64 |     0 |     0 |     0 | 4.002e-06  |            |
| 269, instance   0 (of   1) |     9 |     0 |     0 |     0 | 4.339e-07  |            |
| 270, instance   0 (of   1) |    55 |     0 |     0 |     0 | 2.546e-06  |            |
| 271, instance   0 (of   1) |    83 |     0 |     0 |     0 | 3.272e-06  |            |
| 272, instance   0 (of   1) |   111 |     0 |     0 |     0 | 3.467e-06  |            |
| 273, instance   0 (of   1) |    87 |     0 |     0 |     0 | 2.576e-06  |            |
| 274, instance   0 (of   1) |   135 |     0 |     0 |     0 | 3.106e-06  |            |
| 275, instance   0 (of   1) |   157 |     0 |     0 |     0 | 3.418e-06  |            |
| 276, instance   0 (of   1) |   200 |     0 |     0 |     0 | 3.989e-06  |            |
| 277, instance   0 (of   1) |   188 |     0 |     0 |     0 | 3.252e-06  |            |
| 278, instance   0 (of   1) |   254 |     0 |     0 |     0 | 3.947e-06  |            |
| 279, instance   0 (of   1) |   312 |     0 |     0 |     0 | 3.793e-06  |            |
| 280, instance   0 (of   1) |   320 |     0 |     0 |     0 | 3.673e-06  |            |
| 281, instance   0 (of   1) |   538 |     0 |     0 |     0 | 4.955e-06  |            |
| 282, instance   0 (of   1) |   479 |     0 |     0 |     0 | 3.731e-06  |            |
| 283, instance   0 (of   1) |   310 |     0 |     0 |     0 | 3.326e-06  |            |
| 284, instance   0 (of   1) |   363 |     0 |     0 |     0 | 4.166e-06  |            |
| 285, instance   0 (of   1) |   218 |     0 |     0 |     0 | 2.076e-06  |            |
| 286, instance   0 (of   1) |   251 |     0 |     0 |     0 | 3.406e-06  |            |
| 287, instance   0 (of   1) |   290 |     0 |     0 |     0 | 3.829e-06  |            |
| 288, instance   0 (of   1) |   321 |     0 |     0 |     0 | 3.824e-06  |            |
| 289, instance   0 (of   1) |    15 |     0 |     0 |     0 | 1.034e-06  |            |
| 290, instance   0 (of   1) |    34 |     0 |     0 |     0 | 2.614e-06  |            |
| 291, instance   0 (of   1) |    49 |     0 |     0 |     0 | 3.314e-06  |            |
| 292, instance   0 (of   1) |    66 |     0 |     0 |     0 | 3.761e-06  |            |
| 293, instance   0 (of   1) |     6 |     0 |     0 |     0 | 3.325e-07  |            |
| 294, instance   0 (of   1) |    59 |     0 |     0 |     0 | 2.694e-06  |            |
| 295, instance   0 (of   1) |    85 |     0 |     0 |     0 | 3.312e-06  |            |
| 296, instance   0 (of   1) |   120 |     0 |     0 |     0 | 4.029e-06  |            |
| 297, instance   0 (of   1) |    80 |     0 |     0 |     0 | 2.290e-06  |            |
| 298, instance   0 (of   1) |   118 |     0 |     0 |     0 | 2.857e-06  |            |
| 299, instance   0 (of   1) |   173 |     0 |     0 |     0 | 3.690e-06  |            |
| 300, instance   0 (of   1) |   195 |     0 |     0 |     0 | 3.871e-06  |            |
| 301, instance   0 (of   1) |   196 |     0 |     0 |     0 | 3.214e-06  |            |
| 302, instance   0 (of   1) |   248 |     0 |     0 |     0 | 3.700e-06  |            |
| 303, instance   0 (of   1) |   318 |     0 |     0 |     0 | 4.054e-06  |            |
| 304, instance   0 (of   1) |   318 |     0 |     0 |     0 | 3.626e-06  |            |
| 305, instance   0 (of   1) |   546 |     0 |     0 |     0 | 4.965e-06  |            |
| 306, instance   0 (of   1) |   483 |     0 |     0 |     0 | 3.679e-06  |            |
| 307, instance   0 (of   1) |   301 |     0 |     0 |     0 | 3.378e-06  |            |
| 308, instance   0 (of   1) |   354 |     0 |     0 |     0 | 4.112e-06  |            |
| 309, instance   0 (of   1) |   217 |     0 |     0 |     0 | 1.893e-06  |            |
| 310, instance   0 (of   1) |   252 |     0 |     0 |     0 | 3.408e-06  |            |
| 311, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.968e-06  |            |
| 312, instance   0 (of   1) |   333 |     0 |     0 |     0 | 3.891e-06  |            |
| 313, instance   0 (of   1) |    13 |     0 |     0 |     0 | 9.117e-07  |            |
| 314, instance   0 (of   1) |    25 |     0 |     0 |     0 | 1.916e-06  |            |
| 315, instance   0 (of   1) |    54 |     0 |     0 |     0 | 3.732e-06  |            |
| 316, instance   0 (of   1) |    59 |     0 |     0 |     0 | 3.554e-06  |            |
| 317, instance   0 (of   1) |    25 |     0 |     0 |     0 | 1.322e-06  |            |
| 318, instance   0 (of   1) |    50 |     0 |     0 |     0 | 2.271e-06  |            |
| 319, instance   0 (of   1) |    86 |     0 |     0 |     0 | 3.076e-06  |            |
| 320, instance   0 (of   1) |   123 |     0 |     0 |     0 | 3.915e-06  |            |
| 321, instance   0 (of   1) |    79 |     0 |     0 |     0 | 2.194e-06  |            |
| 322, instance   0 (of   1) |   136 |     0 |     0 |     0 | 3.339e-06  |            |
| 323, instance   0 (of   1) |   168 |     0 |     0 |     0 | 3.603e-06  |            |
| 324, instance   0 (of   1) |   209 |     0 |     0 |     0 | 3.982e-06  |            |
| 325, instance   0 (of   1) |   193 |     0 |     0 |     0 | 3.333e-06  |            |
| 326, instance   0 (of   1) |   255 |     0 |     0 |     0 | 3.838e-06  |            |
| 327, instance   0 (of   1) |   325 |     0 |     0 |     0 | 4.077e-06  |            |
| 328, instance   0 (of   1) |   304 |     0 |     0 |     0 | 3.392e-06  |            |
| 329, instance   0 (of   1) |   563 |     0 |     0 |     0 | 5.186e-06  |            |
| 330, instance   0 (of   1) |   453 |     0 |     0 |     0 | 3.523e-06  |            |
| 331, instance   0 (of   1) |   327 |     0 |     0 |     0 | 3.590e-06  |            |
| 332, instance   0 (of   1) |   350 |     0 |     0 |     0 | 4.103e-06  |            |
| 333, instance   0 (of   1) |   202 |     0 |     0 |     0 | 1.916e-06  |            |
| 334, instance   0 (of   1) |   247 |     0 |     0 |     0 | 3.550e-06  |            |
| 335, instance   0 (of   1) |   301 |     0 |     0 |     0 | 4.004e-06  |            |
| 336, instance   0 (of   1) |   326 |     0 |     0 |     0 | 3.777e-06  |            |
| 337, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.169e-06  |            |
| 338, instance   0 (of   1) |    34 |     0 |     0 |     0 | 2.568e-06  |            |
| 339, instance   0 (of   1) |    46 |     0 |     0 |     0 | 3.072e-06  |            |
| 340, instance   0 (of   1) |    61 |     0 |     0 |     0 | 3.546e-06  |            |
| 341, instance   0 (of   1) |    19 |     0 |     0 |     0 | 1.009e-06  |            |
| 342, instance   0 (of   1) |    68 |     0 |     0 |     0 | 3.129e-06  |            |
| 343, instance   0 (of   1) |    89 |     0 |     0 |     0 | 3.245e-06  |            |
| 344, instance   0 (of   1) |   113 |     0 |     0 |     0 | 3.588e-06  |            |
| 345, instance   0 (of   1) |    62 |     0 |     0 |     0 | 1.584e-06  |            |
| 346, instance   0 (of   1) |   139 |     0 |     0 |     0 | 3.101e-06  |            |
| 347, instance   0 (of   1) |   180 |     0 |     0 |     0 | 3.827e-06  |            |
| 348, instance   0 (of   1) |   207 |     0 |     0 |     0 | 3.731e-06  |            |
| 349, instance   0 (of   1) |   176 |     0 |     0 |     0 | 2.901e-06  |            |
| 350, instance   0 (of   1) |   265 |     0 |     0 |     0 | 3.869e-06  |            |
| 351, instance   0 (of   1) |   333 |     0 |     0 |     0 | 4.226e-06  |            |
| 352, instance   0 (of   1) |   299 |     0 |     0 |     0 | 3.294e-06  |            |
| 353, instance   0 (of   1) |   553 |     0 |     0 |     0 | 4.971e-06  |            |
| 354, instance   0 (of   1) |   475 |     0 |     0 |     0 | 3.628e-06  |            |
| 355, instance   0 (of   1) |   306 |     0 |     0 |     0 | 3.259e-06  |            |
| 356, instance   0 (of   1) |   375 |     0 |     0 |     0 | 4.284e-06  |            |
| 357, instance   0 (of   1) |   200 |     0 |     0 |     0 | 1.874e-06  |            |
| 358, instance   0 (of   1) |   264 |     0 |     0 |     0 | 3.522e-06  |            |
| 359, instance   0 (of   1) |   292 |     0 |     0 |     0 | 3.825e-06  |            |
| 360, instance   0 (of   1) |   319 |     0 |     0 |     0 | 3.776e-06  |            |
| 361, instance   0 (of   1) |    16 |     0 |     0 |     0 | 1.121e-06  |            |
| 362, instance   0 (of   1) |    25 |     0 |     0 |     0 | 1.853e-06  |            |
| 363, instance   0 (of   1) |    51 |     0 |     0 |     0 | 3.442e-06  |            |
| 364, instance   0 (of   1) |    57 |     0 |     0 |     0 | 3.630e-06  |            |
| 365, instance   0 (of   1) |    14 |     0 |     0 |     0 | 7.372e-07  |            |
| 366, instance   0 (of   1) |    57 |     0 |     0 |     0 | 2.639e-06  |            |
| 367, instance   0 (of   1) |    81 |     0 |     0 |     0 | 3.027e-06  |            |
| 368, instance   0 (of   1) |   148 |     0 |     0 |     0 | 4.518e-06  |            |
| 369, instance   0 (of   1) |    75 |     0 |     0 |     0 | 1.830e-06  |            |
| 370, instance   0 (of   1) |   162 |     0 |     0 |     0 | 3.400e-06  |            |
| 371, instance   0 (of   1) |   182 |     0 |     0 |     0 | 3.656e-06  |            |
| 372, instance   0 (of   1) |   189 |     0 |     0 |     0 | 3.484e-06  |            |
| 373, instance   0 (of   1) |   217 |     0 |     0 |     0 | 3.673e-06  |            |
| 374, instance   0 (of   1) |   260 |     0 |     0 |     0 | 3.925e-06  |            |
| 375, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.669e-06  |            |
| 376, instance   0 (of   1) |   334 |     0 |     0 |     0 | 3.774e-06  |            |
| 377, instance   0 (of   1) |   551 |     0 |     0 |     0 | 4.946e-06  |            |
| 378, instance   0 (of   1) |   497 |     0 |     0 |     0 | 3.846e-06  |            |
| 379, instance   0 (of   1) |   317 |     0 |     0 |     0 | 3.254e-06  |            |
| 380, instance   0 (of   1) |   359 |     0 |     0 |     0 | 4.060e-06  |            |
| 381, instance   0 (of   1) |   207 |     0 |     0 |     0 | 1.842e-06  |            |
| 382, instance   0 (of   1) |   258 |     0 |     0 |     0 | 3.629e-06  |            |
| 383, instance   0 (of   1) |   291 |     0 |     0 |     0 | 3.934e-06  |            |
| 384, instance   0 (of   1) |   331 |     0 |     0 |     0 | 3.769e-06  |            |
| 385, instance   0 (of   1) |    12 |     0 |     0 |     0 | 8.395e-07  |            |
| 386, instance   0 (of   1) |    41 |     0 |     0 |     0 | 3.086e-06  |            |
| 387, instance   0 (of   1) |    54 |     0 |     0 |     0 | 3.585e-06  |            |
| 388, instance   0 (of   1) |    63 |     0 |     0 |     0 | 3.722e-06  |            |
| 389, instance   0 (of   1) |    10 |     0 |     0 |     0 | 5.061e-07  |            |
| 390, instance   0 (of   1) |    64 |     0 |     0 |     0 | 2.832e-06  |            |
| 391, instance   0 (of   1) |    74 |     0 |     0 |     0 | 2.870e-06  |            |
| 392, instance   0 (of   1) |   130 |     0 |     0 |     0 | 4.301e-06  |            |
| 393, instance   0 (of   1) |    75 |     0 |     0 |     0 | 2.188e-06  |            |
| 394, instance   0 (of   1) |   138 |     0 |     0 |     0 | 3.413e-06  |            |
| 395, instance   0 (of   1) |   156 |     0 |     0 |     0 | 3.448e-06  |            |
| 396, instance   0 (of   1) |   187 |     0 |     0 |     0 | 3.571e-06  |            |
| 397, instance   0 (of   1) |   198 |     0 |     0 |     0 | 3.390e-06  |            |
| 398, instance   0 (of   1) |   266 |     0 |     0 |     0 | 4.096e-06  |            |
| 399, instance   0 (of   1) |   297 |     0 |     0 |     0 | 3.720e-06  |            |
| 400, instance   0 (of   1) |   295 |     0 |     0 |     0 | 3.440e-06  |            |
| 401, instance   0 (of   1) |   564 |     0 |     0 |     0 | 5.236e-06  |            |
| 402, instance   0 (of   1) |   480 |     0 |     0 |     0 | 3.818e-06  |            |
| 403, instance   0 (of   1) |   320 |     0 |     0 |     0 | 3.387e-06  |            |
| 404, instance   0 (of   1) |   363 |     0 |     0 |     0 | 4.205e-06  |            |
| 405, instance   0 (of   1) |   197 |     0 |     0 |     0 | 1.834e-06  |            |
| 406, instance   0 (of   1) |   260 |     0 |     0 |     0 | 3.571e-06  |            |
| 407, instance   0 (of   1) |   292 |     0 |     0 |     0 | 3.927e-06  |            |
| 408, instance   0 (of   1) |   322 |     0 |     0 |     0 | 3.933e-06  |            |
| 409, instance   0 (of   1) |    18 |     0 |     0 |     0 | 1.278e-06  |            |
| 410, instance   0 (of   1) |    18 |     0 |     0 |     0 | 1.357e-06  |            |
| 411, instance   0 (of   1) |    47 |     0 |     0 |     0 | 3.282e-06  |            |
| 412, instance   0 (of   1) |    51 |     0 |     0 |     0 | 3.170e-06  |            |
| 413, instance   0 (of   1) |    24 |     0 |     0 |     0 | 1.545e-06  |            |
| 414, instance   0 (of   1) |    51 |     0 |     0 |     0 | 2.412e-06  |            |
| 415, instance   0 (of   1) |    79 |     0 |     0 |     0 | 3.103e-06  |            |
| 416, instance   0 (of   1) |   112 |     0 |     0 |     0 | 3.811e-06  |            |
| 417, instance   0 (of   1) |    74 |     0 |     0 |     0 | 2.176e-06  |            |
| 418, instance   0 (of   1) |   140 |     0 |     0 |     0 | 3.220e-06  |            |
| 419, instance   0 (of   1) |   173 |     0 |     0 |     0 | 3.832e-06  |            |
| 420, instance   0 (of   1) |   188 |     0 |     0 |     0 | 3.548e-06  |            |
| 421, instance   0 (of   1) |   193 |     0 |     0 |     0 | 3.388e-06  |            |
| 422, instance   0 (of   1) |   244 |     0 |     0 |     0 | 3.823e-06  |            |
| 423, instance   0 (of   1) |   312 |     0 |     0 |     0 | 4.045e-06  |            |
| 424, instance   0 (of   1) |   297 |     0 |     0 |     0 | 3.460e-06  |            |
| 425, instance   0 (of   1) |   539 |     0 |     0 |     0 | 4.810e-06  |            |
| 426, instance   0 (of   1) |   479 |     0 |     0 |     0 | 3.716e-06  |            |
| 427, instance   0 (of   1) |   327 |     0 |     0 |     0 | 3.492e-06  |            |
| 428, instance   0 (of   1) |   355 |     0 |     0 |     0 | 4.025e-06  |            |
| 429, instance   0 (of   1) |   195 |     0 |     0 |     0 | 1.898e-06  |            |
| 430, instance   0 (of   1) |   245 |     0 |     0 |     0 | 3.236e-06  |            |
| 431, instance   0 (of   1) |   297 |     0 |     0 |     0 | 4.042e-06  |            |
| 432, instance   0 (of   1) |   333 |     0 |     0 |     0 | 3.938e-06  |            |
| 433, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.227e-06  |            |
| 434, instance   0 (of   1) |    26 |     0 |     0 |     0 | 1.946e-06  |            |
| 435, instance   0 (of   1) |    48 |     0 |     0 |     0 | 3.472e-06  |            |
| 436, instance   0 (of   1) |    64 |     0 |     0 |     0 | 4.115e-06  |            |
| 437, instance   0 (of   1) |     4 |     0 |     0 |     0 | 2.081e-07  |            |
| 438, instance   0 (of   1) |    48 |     0 |     0 |     0 | 2.390e-06  |            |
| 439, instance   0 (of   1) |    75 |     0 |     0 |     0 | 3.391e-06  |            |
| 440, instance   0 (of   1) |    86 |     0 |     0 |     0 | 3.431e-06  |            |
| 441, instance   0 (of   1) |    83 |     0 |     0 |     0 | 2.691e-06  |            |
| 442, instance   0 (of   1) |   106 |     0 |     0 |     0 | 2.915e-06  |            |
| 443, instance   0 (of   1) |   146 |     0 |     0 |     0 | 3.473e-06  |            |
| 444, instance   0 (of   1) |   189 |     0 |     0 |     0 | 3.904e-06  |            |
| 445, instance   0 (of   1) |   197 |     0 |     0 |     0 | 3.544e-06  |            |
| 446, instance   0 (of   1) |   238 |     0 |     0 |     0 | 3.785e-06  |            |
| 447, instance   0 (of   1) |   315 |     0 |     0 |     0 | 3.935e-06  |            |
| 448, instance   0 (of   1) |   319 |     0 |     0 |     0 | 3.740e-06  |            |
| 449, instance   0 (of   1) |   528 |     0 |     0 |     0 | 4.818e-06  |            |
| 450, instance   0 (of   1) |   494 |     0 |     0 |     0 | 3.967e-06  |            |
| 451, instance   0 (of   1) |   306 |     0 |     0 |     0 | 3.265e-06  |            |
| 452, instance   0 (of   1) |   366 |     0 |     0 |     0 | 4.050e-06  |            |
| 453, instance   0 (of   1) |   206 |     0 |     0 |     0 | 1.904e-06  |            |
| 454, instance   0 (of   1) |   264 |     0 |     0 |     0 | 3.700e-06  |            |
| 455, instance   0 (of   1) |   297 |     0 |     0 |     0 | 3.914e-06  |            |
| 456, instance   0 (of   1) |   323 |     0 |     0 |     0 | 3.805e-06  |            |
| 457, instance   0 (of   1) |    21 |     0 |     0 |     0 | 1.580e-06  |            |
| 458, instance   0 (of   1) |    22 |     0 |     0 |     0 | 1.752e-06  |            |
| 459, instance   0 (of   1) |    45 |     0 |     0 |     0 | 3.298e-06  |            |
| 460, instance   0 (of   1) |    69 |     0 |     0 |     0 | 4.424e-06  |            |
| 461, instance   0 (of   1) |    12 |     0 |     0 |     0 | 6.113e-07  |            |
| 462, instance   0 (of   1) |    55 |     0 |     0 |     0 | 2.798e-06  |            |
| 463, instance   0 (of   1) |    74 |     0 |     0 |     0 | 3.184e-06  |            |
| 464, instance   0 (of   1) |   105 |     0 |     0 |     0 | 3.876e-06  |            |
| 465, instance   0 (of   1) |    76 |     0 |     0 |     0 | 2.409e-06  |            |
| 466, instance   0 (of   1) |   111 |     0 |     0 |     0 | 2.680e-06  |            |
| 467, instance   0 (of   1) |   165 |     0 |     0 |     0 | 3.794e-06  |            |
| 468, instance   0 (of   1) |   189 |     0 |     0 |     0 | 3.845e-06  |            |
| 469, instance   0 (of   1) |   182 |     0 |     0 |     0 | 3.280e-06  |            |
| 470, instance   0 (of   1) |   237 |     0 |     0 |     0 | 3.815e-06  |            |
| 471, instance   0 (of   1) |   296 |     0 |     0 |     0 | 3.759e-06  |            |
| 472, instance   0 (of   1) |   309 |     0 |     0 |     0 | 3.685e-06  |            |
| 473, instance   0 (of   1) |   518 |     0 |     0 |     0 | 4.896e-06  |            |
| 474, instance   0 (of   1) |   474 |     0 |     0 |     0 | 3.792e-06  |            |
| 475, instance   0 (of   1) |   295 |     0 |     0 |     0 | 3.328e-06  |            |
| 476, instance   0 (of   1) |   354 |     0 |     0 |     0 | 4.086e-06  |            |
| 477, instance   0 (of   1) |   201 |     0 |     0 |     0 | 1.794e-06  |            |
| 478, instance   0 (of   1) |   248 |     0 |     0 |     0 | 3.351e-06  |            |
| 479, instance   0 (of   1) |   292 |     0 |     0 |     0 | 3.934e-06  |            |
| 480, instance   0 (of   1) |   323 |     0 |     0 |     0 | 3.799e-06  |            |
| 481, instance   0 (of   1) |    25 |     0 |     0 |     0 | 1.810e-06  |            |
| 482, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.336e-06  |            |
| 483, instance   0 (of   1) |    50 |     0 |     0 |     0 | 3.693e-06  |            |
| 484, instance   0 (of   1) |    54 |     0 |     0 |     0 | 3.705e-06  |            |
| 485, instance   0 (of   1) |     8 |     0 |     0 |     0 | 5.995e-07  |            |
| 486, instance   0 (of   1) |    44 |     0 |     0 |     0 | 2.455e-06  |            |
| 487, instance   0 (of   1) |    70 |     0 |     0 |     0 | 3.302e-06  |            |
| 488, instance   0 (of   1) |   113 |     0 |     0 |     0 | 4.235e-06  |            |
| 489, instance   0 (of   1) |    65 |     0 |     0 |     0 | 1.903e-06  |            |
| 490, instance   0 (of   1) |   136 |     0 |     0 |     0 | 3.417e-06  |            |
| 491, instance   0 (of   1) |   148 |     0 |     0 |     0 | 3.206e-06  |            |
| 492, instance   0 (of   1) |   191 |     0 |     0 |     0 | 3.778e-06  |            |
| 493, instance   0 (of   1) |   199 |     0 |     0 |     0 | 3.506e-06  |            |
| 494, instance   0 (of   1) |   254 |     0 |     0 |     0 | 3.923e-06  |            |
| 495, instance   0 (of   1) |   303 |     0 |     0 |     0 | 3.838e-06  |            |
| 496, instance   0 (of   1) |   315 |     0 |     0 |     0 | 3.665e-06  |            |
| 497, instance   0 (of   1) |   517 |     0 |     0 |     0 | 4.707e-06  |            |
| 498, instance   0 (of   1) |   489 |     0 |     0 |     0 | 3.905e-06  |            |
| 499, instance   0 (of   1) |   298 |     0 |     0 |     0 | 3.414e-06  |            |
| 500, instance   0 (of   1) |   357 |     0 |     0 |     0 | 4.159e-06  |            |
| 501, instance   0 (of   1) |   205 |     0 |     0 |     0 | 1.917e-06  |            |
| 502, instance   0 (of   1) |   248 |     0 |     0 |     0 | 3.503e-06  |            |
| 503, instance   0 (of   1) |   304 |     0 |     0 |     0 | 3.999e-06  |            |
| 504, instance   0 (of   1) |   333 |     0 |     0 |     0 | 3.816e-06  |            |
| 505, instance   0 (of   1) |    29 |     0 |     0 |     0 | 2.006e-06  |            |
| 506, instance   0 (of   1) |    15 |     0 |     0 |     0 | 1.124e-06  |            |
| 507, instance   0 (of   1) |    37 |     0 |     0 |     0 | 2.707e-06  |            |
| 508, instance   0 (of   1) |    58 |     0 |     0 |     0 | 4.003e-06  |            |
| 509, instance   0 (of   1) |     7 |     0 |     0 |     0 | 4.660e-07  |            |
| 510, instance   0 (of   1) |    56 |     0 |     0 |     0 | 3.108e-06  |            |
| 511, instance   0 (of   1) |    67 |     0 |     0 |     0 | 3.121e-06  |            |
| 512, instance   0 (of   1) |    93 |     0 |     0 |     0 | 3.593e-06  |            |
| 513, instance   0 (of   1) |    82 |     0 |     0 |     0 | 2.651e-06  |            |
| 514, instance   0 (of   1) |   129 |     0 |     0 |     0 | 3.189e-06  |            |
| 515, instance   0 (of   1) |   155 |     0 |     0 |     0 | 3.537e-06  |            |
| 516, instance   0 (of   1) |   189 |     0 |     0 |     0 | 3.614e-06  |            |
| 517, instance   0 (of   1) |   203 |     0 |     0 |     0 | 3.603e-06  |            |
| 518, instance   0 (of   1) |   243 |     0 |     0 |     0 | 3.789e-06  |            |
| 519, instance   0 (of   1) |   303 |     0 |     0 |     0 | 3.914e-06  |            |
| 520, instance   0 (of   1) |   316 |     0 |     0 |     0 | 3.699e-06  |            |
| 521, instance   0 (of   1) |   536 |     0 |     0 |     0 | 4.933e-06  |            |
| 522, instance   0 (of   1) |   471 |     0 |     0 |     0 | 3.760e-06  |            |
| 523, instance   0 (of   1) |   320 |     0 |     0 |     0 | 3.572e-06  |            |
| 524, instance   0 (of   1) |   343 |     0 |     0 |     0 | 3.821e-06  |            |
| 525, instance   0 (of   1) |   204 |     0 |     0 |     0 | 1.869e-06  |            |
| 526, instance   0 (of   1) |   257 |     0 |     0 |     0 | 3.533e-06  |            |
| 527, instance   0 (of   1) |   297 |     0 |     0 |     0 | 3.967e-06  |            |
| 528, instance   0 (of   1) |   311 |     0 |     0 |     0 | 3.720e-06  |            |
| 529, instance   0 (of   1) |    21 |     0 |     0 |     0 | 1.544e-06  |            |
| 530, instance   0 (of   1) |    18 |     0 |     0 |     0 | 1.375e-06  |            |
| 531, instance   0 (of   1) |    58 |     0 |     0 |     0 | 4.390e-06  |            |
| 532, instance   0 (of   1) |    53 |     0 |     0 |     0 | 3.618e-06  |            |
| 533, instance   0 (of   1) |    10 |     0 |     0 |     0 | 6.851e-07  |            |
| 534, instance   0 (of   1) |    56 |     0 |     0 |     0 | 2.796e-06  |            |
| 535, instance   0 (of   1) |    71 |     0 |     0 |     0 | 3.110e-06  |            |
| 536, instance   0 (of   1) |    85 |     0 |     0 |     0 | 3.253e-06  |            |
| 537, instance   0 (of   1) |    77 |     0 |     0 |     0 | 2.534e-06  |            |
| 538, instance   0 (of   1) |   105 |     0 |     0 |     0 | 2.904e-06  |            |
| 539, instance   0 (of   1) |   152 |     0 |     0 |     0 | 3.504e-06  |            |
| 540, instance   0 (of   1) |   163 |     0 |     0 |     0 | 3.465e-06  |            |
| 541, instance   0 (of   1) |   200 |     0 |     0 |     0 | 3.548e-06  |            |
| 542, instance   0 (of   1) |   235 |     0 |     0 |     0 | 3.727e-06  |            |
| 543, instance   0 (of   1) |   301 |     0 |     0 |     0 | 4.034e-06  |            |
| 544, instance   0 (of   1) |   293 |     0 |     0 |     0 | 3.563e-06  |            |
| 545, instance   0 (of   1) |   511 |     0 |     0 |     0 | 4.901e-06  |            |
| 546, instance   0 (of   1) |   446 |     0 |     0 |     0 | 3.872e-06  |            |
| 547, instance   0 (of   1) |   287 |     0 |     0 |     0 | 3.577e-06  |            |
| 548, instance   0 (of   1) |   320 |     0 |     0 |     0 | 3.860e-06  |            |
| 549, instance   0 (of   1) |   187 |     0 |     0 |     0 | 1.907e-06  |            |
| 550, instance   0 (of   1) |   228 |     0 |     0 |     0 | 3.349e-06  |            |
| 551, instance   0 (of   1) |   281 |     0 |     0 |     0 | 4.059e-06  |            |
| 552, instance   0 (of   1) |   309 |     0 |     0 |     0 | 3.884e-06  |            |
| 553, instance   0 (of   1) |    19 |     0 |     0 |     0 | 1.410e-06  |            |
| 554, instance   0 (of   1) |    24 |     0 |     0 |     0 | 2.012e-06  |            |
| 555, instance   0 (of   1) |    38 |     0 |     0 |     0 | 3.107e-06  |            |
| 556, instance   0 (of   1) |    42 |     0 |     0 |     0 | 3.327e-06  |            |
| 557, instance   0 (of   1) |    17 |     0 |     0 |     0 | 1.462e-06  |            |
| 558, instance   0 (of   1) |    36 |     0 |     0 |     0 | 2.314e-06  |            |
| 559, instance   0 (of   1) |    55 |     0 |     0 |     0 | 2.892e-06  |            |
| 560, instance   0 (of   1) |    71 |     0 |     0 |     0 | 3.626e-06  |            |
| 561, instance   0 (of   1) |    62 |     0 |     0 |     0 | 2.928e-06  |            |
| 562, instance   0 (of   1) |    78 |     0 |     0 |     0 | 2.965e-06  |            |
| 563, instance   0 (of   1) |   104 |     0 |     0 |     0 | 3.801e-06  |            |
| 564, instance   0 (of   1) |   106 |     0 |     0 |     0 | 3.587e-06  |            |
| 565, instance   0 (of   1) |   122 |     0 |     0 |     0 | 3.520e-06  |            |
| 566, instance   0 (of   1) |   166 |     0 |     0 |     0 | 3.895e-06  |            |
| 567, instance   0 (of   1) |   204 |     0 |     0 |     0 | 4.026e-06  |            |
| 568, instance   0 (of   1) |   235 |     0 |     0 |     0 | 4.004e-06  |            |
| 569, instance   0 (of   1) |   345 |     0 |     0 |     0 | 4.074e-06  |            |
| 570, instance   0 (of   1) |   389 |     0 |     0 |     0 | 4.032e-06  |            |
| 571, instance   0 (of   1) |   262 |     0 |     0 |     0 | 3.518e-06  |            |
| 572, instance   0 (of   1) |   285 |     0 |     0 |     0 | 3.836e-06  |            |
| 573, instance   0 (of   1) |   143 |     0 |     0 |     0 | 1.638e-06  |            |
| 574, instance   0 (of   1) |   225 |     0 |     0 |     0 | 3.914e-06  |            |
| 575, instance   0 (of   1) |   236 |     0 |     0 |     0 | 4.346e-06  |            |
| 576, instance   0 (of   1) |   248 |     0 |     0 |     0 | 3.484e-06  |            |
| 577, instance   0 (of   1) |  8562 |     0 |     0 |     0 | 6.139e-04  |            |
| 579, instance   0 (of   1) | 53583 |     0 |     0 |     0 | 9.319e-04  |            |
| 580, instance   0 (of   1) | 53979 |     0 |     0 |     0 | 9.304e-04  |            |
| 613, instance   0 (of   1) | 42663 |     0 |     0 |     0 | 3.465e-04  |            |
| 614, instance   0 (of   1) | 50338 |     0 |     0 |     0 | 5.789e-04  |            |
| 615, instance   0 (of   1) | 38227 |     0 |     0 |     0 | 2.942e-04  |            |
| 616, instance   0 (of   1) | 34153 |     0 |     0 |     0 | 2.497e-04  |            |
| 617, instance   0 (of   1) | 29815 |     0 |     0 |     0 | 6.205e-04  |            |
| 618, instance   0 (of   1) | 24836 |     0 |     0 |     0 | 8.822e-04  |            |
| 619, instance   0 (of   1) | 21965 |     0 |     0 |     0 | 1.981e-04  |            |
| 620, instance   0 (of   1) | 21483 |     0 |     0 |     0 | 2.256e-04  |            |
----------------------------------------------------------------------------------------

 ===================>     AUXVARIABLES FOR OPENMC I/O     <===================

      Subdomain:  subdomain name/ID
    Temperature:  variable OpenMC reads temperature from (empty if no feedback)
        Density:  variable OpenMC reads density from (empty if no feedback)

-----------------------------------------
|   Subdomain   | Temperature | Density |
-----------------------------------------
| gap_clamp     | temp        |         |
| clamp         | temp        |         |
| gap_vaccan    | temp        |         |
| MLI           | temp        |         |
| vacuum_can    | temp        |         |
| gap_sleeve    | temp        |         |
| sleeve        | temp        |         |
| gap_ref       | temp        |         |
| cavity_center | temp        |         |
| ref_bottom    | temp        |         |
| core          | temp        |         |
| ref_top       | temp        |         |
-----------------------------------------

    Tally Score:  OpenMC tally score
    AuxVariable:  variable where this score is written

-------------------------------
|  Tally Score  | AuxVariable |
-------------------------------
| kappa-fission | heat_source |
-------------------------------

 ===================>  OPENMC SUBDOMAIN MATERIAL MAPPING  <====================

      Subdomain:  Subdomain name; if unnamed, we show the ID
       Material:  OpenMC material name(s) in this subdomain; if unnamed, we
                  show the ID. If N duplicate material names, we show the
                  number in ( ).

-----------------------------------
|   Subdomain   |    Material     |
-----------------------------------
| gap_clamp     |  Helium for gap |
| clamp         |  Haynes230      |
| gap_vaccan    |  Helium for gap |
| MLI           |  Mo Foil        |
| vacuum_can    |  SS             |
| gap_sleeve    |  Helium for gap |
| sleeve        |  SS             |
| gap_ref       |  Helium for gap |
| cavity_center |  VOID           |
| ref_bottom    |  BeO reflector  |
| core          |  U-7.65Mo Fuel  |
| ref_top       |  BeO reflector  |

```

This shows the OpenMC cells mapped to the MOOSE elements. The above message also shows the
volume that each OpenMC cell maps to. Because there are no distributed cells in this
problem, each cell only has a single instance. Since we added a stochastic volume
calculation, the last column (`Actual Vol`) is populated with OpenMC's stochastic
estimates for the cell volumes. You can increase the number of samples to drive the error
lower to get more refined estimates of volumes. We cab also see the auxiliary variables and material mapping for OpenMC  

[core_height] shows the heat source mapped along the KRUSTY core height.

!media core_height.png
  id=core_height
  caption=Heat source (W/cm$^3$) computed by OpenMC
  style=width:45%;

[temp_krusty] and [power_krusty] shows the multiphysics results for temperature and power density.

!media temp_krusty.png
  id=temp_krusty
  caption=Multiphysics results for temperature (K)
  style=width:45%;float:left

!media power_krusty.png
  id=power_krusty
  caption=Multiphysics results for power density (W/cm$^3$)
  style=width:45%;float:right
