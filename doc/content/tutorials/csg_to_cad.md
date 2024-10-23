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

### MOOSE Heat Conduction

### OpenMC Neutron Transport

## Execution and Postprocessing

To run the coupled calculation,

```
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

This will run both MOOSE and OpenMC with 2 MPI processes and 2 OpenMP threads per rank.
To run the simulation faster, you can increase the parallel processes/threads, or
simply decrease the number of particles used in OpenMC.
When the simulation has completed, you will have created a number of different output files:
