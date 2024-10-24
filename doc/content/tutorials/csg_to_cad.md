# Converting CSG to CAD Geometry for Multiphysics

In this tutorial, you will learn how to:

- Convert OpenMC CSG-based models to DAGMC using the Coreform Cubit CSG-to-CAD converter
- Perform multiphysics feedback on a portion of the OpenMC model by coupling to finite element heat conduction
- Run hybrid CSG + DAGMC OpenMC models with Cardinal

To access this tutorial,

```
cd cardinal/tutorials/csg_to_cad
```

This tutorial also requires you to download
some mesh files from Box. Please download the files
from the `csg_to_cad` folder [here](https://anl.app.box.com/folder/290460582816)
and place these files within `tutorials/csg_to_cad`.

!alert! note title=DAGMC build
To run this tutorial, you need to have built Cardinal with DAGMC support enabled, by setting
`export ENABLE_DAGMC=true`.
!alert-end!

## Geometry and Computational Models

This model consists of a simplified version of the KRUSTY reactor.

## Converting CSG to CAD using Coreform Cubit

## Multiphysics Coupling

We have built a hybrid [!ac](CAD) and [!ac](CSG) model of the KRUSTY reactor in OpenMC,
along with a volume mesh for tallying and solving heat conduction. In this section,
we briefly describe the input files used to couple heat conduction to neutronics.

### MOOSE Heat Conduction

We solve for the fuel temperature using the finite element method using MOOSE.
We will solve using the volume mesh produced from Cubit. This mesh is identical
to the mesh which OpenMC will use for tallying, but does not striclty need to be
(it can be entirely different, and most other tutorials demonstrate this feature). But
in this case, we will for simplicity use an identical mesh.

!listing /tutorials/csg_to_cad/fuel.i
  block=Mesh

We will solve for temperature, with a variable `T`. The power density will be
supplied by OpenMC, so we create a variable named `power` in this file in order
to receive that field. The OpenMC tally will be a constant value in every mesh
element, so we define this variable to match this basis.

!listing /tutorials/csg_to_cad/fuel.i
  start=Variables
  end=Kernels

Next, we specify the governing equation and boundary conditions. We will solve
for the steady-state temperature distribution

\begin{equation}
-\nabla\cdot k\nabla T=\dot{q}
\end{equation}

We use the [HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html) and [CoupledForce](https://mooseframework.inl.gov/source/kernels/CoupledForce.html) kernels to define the Laplacian
kernel and the coupled power term, respectively. For boundary conditions, we apply
a constant temperature of 800 on the surface of the heat pipes.

!listing /tutorials/csg_to_cad/fuel.i
  start=Kernels
  end=Postprocessors

Lastly, we need to specify a value for the thermal conductivity. We will set this
to a constant value, for simplicity.

!listing /tutorials/csg_to_cad/fuel.i
  block=Materials

Finally, we need to specify how to solve this equation. We will use a transient
executioner, which will allow us to solve the equation multiple times. We also
indicate the output file format (exodus). For the sake of normalizing the power
we receive from OpenMC, we also add a postprocessor to compute the total integral
of power - while we don't strictly need this because our meshes are identical
between OpenMC and MOOSE for this problem, having this block here would be necessary
if OpenMC and MOOSE were using different meshes, to guarantee power conservation.

!listing /tutorials/csg_to_cad/fuel.i
  start=Postprocessors

### OpenMC Neutron Transport

Our input file to run OpenMC within a MOOSE simulation will look similar to
the previous input file syntax. First, we now have a problem block to inform
MOOSE to replace it's typical finite element solver with calls to OpenMC
$k$-eigenvalue runs. We will have temperature feedback to OpenMC on block 1,
and we add two mesh tallies. This will tally the `kappa_fission` local power
deposition and the `flux` from OpenMC and map them to the mesh used in the mesh block. Other settings in the problem block refer to how to normalize the tallies
from OpenMC into meaningful engineering units (W/volume for heating terms,
and neutrons/area/time for flux).

!listing /tutorials/csg_to_cad/openmc.i
  end=UserObjects

We will dynamically modify the OpenMC geometry by "skinning" with the
[MoabSkinner](https://cardinal.cels.anl.gov/source/userobjects/MoabSkinner.html) object. For simplicity, we will lump elements into new cells by contouring into 4 intervals between temperatures of 800 K and 1000 K.

!listing /tutorials/csg_to_cad/openmc.i
  block=UserObjects

Next, we specify how to pass data between OpenMC and the finite element heat
conduction solver in the `fuel.i` input file. We will run the heat conduction solver
as a sub-application. On every time step, we will pass temperature (into OpenMC)
and the heating tally (out of OpenMC) as listed in the transfers block. The
other details listed in this section are optimizations for faster transfers
(`search_value_conflicts = false`) and details on the source/receiver variable
names in each file and which postprocessors to use to ensure power conservation.

!listing /tutorials/csg_to_cad/openmc.i
  start=MultiApps
  end=ICs

Next, we set some initial conditions, since OpenMC will run first. We set a
constant initial temperature of 800 K. We also add an auxiliary variable,
[CellTemperatureAux](https://cardinal.cels.anl.gov/source/auxkernels/CellTemperatureAux.html), so that we can see the temperature field actually applied to OpenMC
(this will hold the cell temperatures). Lastly, we indicate the run settings -
we will run OpenMC three times, and output all results to Exodus.

!listing /tutorials/csg_to_cad/openmc.i
  start=ICs

## Execution and Postprocessing

To run the coupled calculation,

```
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

This will run both MOOSE and OpenMC with 2 MPI processes and 2 OpenMP threads per rank.
To run the simulation faster, you can increase the parallel processes/threads, or
simply decrease the number of particles used in OpenMC.
When the simulation has completed, you will have created a number of different output files:

- `openmc_out.e`: Exodus output file with all variables in the `openmc.i` file
- `openmc_out_conduction0.e`: Exodus output file with all variables in the `fuel.i` file

!media krusty_results.png
  id=krusty_results
  caption=Solid temperature (left), and OpenMC predictions for neutron flux (middle) and fission heating (right). This simulation is run with an increased number of particles compared to the tutorial files in order to obtain well-converged results.
  style=width:80%;margin-left:auto;margin-right:auto;halign:center
