# Multiphysics Coupling Model of KRUSTY

In this tutorial, you will learn two different cases:

- Case A: couple OpenMC via temperature and heat source feedback to MOOSE for the KRUSTY reactor
- Case B: couple OpenMC via temperature and heat source feedback to MOOSE that is coupled to Sockeye via heat pipes temperature and heat flux feedback for the KRUSTY reactor



In [krusty_caseA] and [krusty_caseB] we can see the multi-application data transfer used in both cases.

!media krusty_caseA.png
  id=krusty_caseA
  caption=Multiapps hierarchy of the KRUSTY model for Case A
  style=width:40%;margin-left:auto;margin-right:auto;halign:center
  
!media krusty_caseB.png
  id=krusty_caseB
  caption=Multiapps hierarchy of the KRUSTY model for Case B
  style=width:40%;halign:center

You can access both cases using the following links:

- [Case A](KRUSTY.md)
- [Case B](KRUSTYB.md)

Next, we are going to present the geometry, the MOOSE heat conduction model, and the OpenMC model, which are common for both cases.

## Geometry

This model consists of a multiphysics simulation of the KRUSTY reactor, one of the simplest space power reactor concepts ever proposed. The relevant dimensions are summarized in [table1] and in [krusty_solid_mesh] shows the geometry and gap thicknesses. The basic components are the fuel, heat pipes, control, reflectors and shield. For the sake of simplification, we will only simulate the components within the region spanning from the bottom axial reflector to the top axial reflector along the z-direction, which entails neglecting components outside of this region, the control rod insertion is also being disregarded. The fuel is high enriched uranium U-7.65 Mo with a total length of 25 cm, an outer diameter of 11 cm, and the core contains a 4 cm hole to allow $B_4C$ control insertion. We can also briefly discuss the basic thermal cycle of a heat pipe, which is described in [fig_heat_pipe].

!media fig_heat_pipe.png
  id=fig_heat_pipe
  caption=KRUSTY geometry schematic [!cite](wikiHP).
  style=width:45%;margin-left:auto;margin-right:auto;halign:center
where $1$ shows the working fluid evaporating to vapour by absorbing thermal energy, at $2$ the vapor migrates along the cavity to lower temperature end, and at $3$ the vapour condenses back to liquid and is absorbed by the wick, releasing thermal energy. Finally, at $4$ the working fluid flows back to the higher temperature end. The following sub-sections will describe the input file for each application.

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
  style=width:45%;margin-left:auto;margin-right:auto;halign:center

## MOOSE Heat Conduction Model

The MOOSE heat transfer module is used to solve for
[energy conservation in the solid](theory/heat_eqn.md).

[krusty_mesh] shows the mesh used in this model. The geometry was built using FUSION360, where the CAD file was imported into Cubit, where the mesh was build. Cubit can create meshes with user-defined geometry and customizable boundary layers. The Cubit script used to generate the solid mesh can be found in the `meshes` directory, where you will find two files, a `.sat` file that you will import into cubit, which will generate the geometry of KRUSTY, however in order to generete the mesh you will need to run the scripts in the `.txt` file in the cubit terminal, and then you can export the `.e` mesh file, which is the format that MOOSE can read. You can find the `.e` files in this box link: [box](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499). Do not forget to place the `.e` mesh files in the same directory as your `.i` files.

!media krusty_mesh.png
  id=krusty_mesh
  caption=KRUSTY mesh
  style=width:45%;margin-left:auto;margin-right:auto;halign:center

## OpenMC Model

The OpenMC model is built using a [!ac](CSG)-type geometry. In a [!ac](CSG) model, cells are created from regions of space formed by half-space of various common surfaces, where every surface divides all of space into exactly two half-spaces. First, we define materials for each component of the reactor. Next, we define our geometric parameters, so we can start building our surfaces and manipulate them to create the geometry of each component of the KRUSTY reactor. Then, we need to create cells for each component and fill them with the materials that we previously created, so we can create a universe containg all the cells using `openmc.Universe`. We can visualize our geometry by slicing our plots in OpenMC, [krusty_openmc_xy] shows a xy slice, and [krusty_openmc_xz] a xz slice of the geometry plot.

!media krusty_openmc_xy.png
  id=krusty_openmc_xy
  caption=OpenMC xy slice plot.
  style=width:50%;float:left;halign:center

!media krusty_openmc_xz.png
  id=krusty_openmc_xz
  caption=OpenMC xz slice plot.
  style=width:50%;float:right;halign:center


We will use a linear-linear stochastic interpolation between the two cross section data sets nearest to the imposed temperature by setting the `method` parameter on `settings.temperature` to `interpolation`. When OpenMC is initialized, the cross section data is loaded for a temperature range specified by `range` in `settings`. 

!listing /tutorials/krusty/KRUSTY/model.py

To generate the all of the XML files needed to run OpenMC, you just need to run: 

```
model.export_to_xml()
```

