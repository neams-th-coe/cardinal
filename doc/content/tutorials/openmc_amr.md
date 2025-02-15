# Adaptive Mesh Refinement on Mesh Tallies

In this tutorial, you will learn how to:

- Generate assembly meshes using the MOOSE [Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html)
- Couple OpenMC to MOOSE (via a heat source) for a single assembly in the C5G7 [!ac](LWR) reactor physics benchmark
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

This model consists of a single UO$_2$ assembly from the C5G7 [!ac](LWR) 3D extension case [!cite](c5g7), where control
rods are fully inserted in the assembly to induce strong axial annd radial gradients. Instead of using the multi-group
cross sections from the C5G7 benchmark specifications, the material properties used to generate the multi-group cross
sections are used. The geometry of this problem is shown below.



### OpenMC Model

### Neutronics Input Files

## Execution and Postprocessing

## Adding Adaptive Mesh Refinement
