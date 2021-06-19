# Tutorial 1A: Conjugate Heat Transfer for Reflector Bypass Flow
  id=tutorial1a

In this tutorial, you will learn how to:

- Couple nekRS with MOOSE for conjugate heat transfer
- Solve nekRS in non-dimensional form while MOOSE solves in dimensional form

This tutorial provides a description of how to use Cardinal to perform
conjugate heat transfer coupling of nekRS to MOOSE for bypass flow modeling in
a portion of a [!ac](PB-FHR) reflector block.
This tutorial was developed as part of the
[!ac](VTB), a [!ac](NRIC) initiative aimed at facilitating the use of advanced modeling
and simulation tools, but has been modified slightly to better fit
the context of a Cardinal tutorial. This particular example uses a very large mesh,
and therefore necessitates [!ac](HPC) resources to run; nevertheless, we begin
with this tutorial for conjugate heat transfer coupling because it is very comprehensive.
If you do not have access to [!ac](HPC) resources, you will still
find the analysis procedure useful in understanding the MOOSE-wrapped app concept
and opportunities for high-resolution thermal-hydraulics within the MOOSE framework.
You will be able to run [#tutorial1b] without [!ac](HPC) resources.

The pebble region in the [!ac](PB-FHR) is enclosed by an outer graphite reflector
that constrains the pebble geometry while also
serving as a reflector for neutrons and a shield for
the reactor barrel and core externals. In order to keep the graphite reflector within
allowable design temperatures, the reflector contains several bypass flow
paths so that a small percentage of the coolant flow, usually on the
order of 5 to 10% of the total flow, can remove heat from the reflector
and maintain the graphite within allowable temperature ranges. This
bypass flow, so-named because coolant is diverted from the pebble region,
is important to quantify during the reactor design process so that accurate
estimates of core cooling and reflector temperatures can be obtained. By repeating
these calculations for a range of Reynolds numbers and geometries, this model
can be used to provide friction factor and Nusselt number correlations as inputs
to a full-core Pronghorn model.

!alert warning
While this tutorial shows analysis of an open-source reactor design, the purpose
of this tutorial is purely pedagogical - many simplifications are made to the model
setup so that the emphasis remains as an instructional example. Please be aware that
the predictions made by this model will not be reflective of the real reactor system
due to these simplifications.

At a high level, Cardinal's wrapping of nekRS consists of:

- Constructing a "mirror" of the nekRS mesh through which data transfers occur
  with MOOSE. For conjugate heat transfer applications, a
  [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
   is created by copying the nekRS surface mesh into a format that
  all native MOOSE applications can understand.
- Adding [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  to represent the nekRS solution. In other words,
  if nekRS stores the temperature internally as an `std::vector<double>`, with each
  entry corresponding to a nekRS node, then a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
   is created that represents
  the same data, but that can be accessed in relation to the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
   mirror.
- Writing multiphysics feedback fields in/out of nekRS's internal solution and boundary
  condition arrays. So, if nekRS represents a heat flux boundary condition internally
  as an `std::vector<double>`, this involves reading from a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html) representing
  heat flux (which can be transferred with any of MOOSE's transfers to the nekRS
  wrapping) and writing into nekRS's internal vectors.

Accomplishing the above three tasks requires an intimate knowledge of how nekRS
stores its solution fields and mesh. But once the wrapping is constructed, nekRS can
then communicate with any other MOOSE application via the [MultiApp]((https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html) systems
in MOOSE, enabling complex multiscale thermal-hydraulic analysis and multiphysics feedback.
The same wrapping can be used for conjugate heat transfer analysis with *any* MOOSE
application that can compute a heat flux; that is, because a MOOSE-wrapped version of nekRS
interacts with the MOOSE framework in a similar manner as natively-developed
MOOSE applications, the agnostic formulations of the [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html) and
[Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) can be
used to equally extract heat flux from Pronghorn, BISON, the MOOSE heat conduction
module, and so on. For pedagogical purposes, this tutorial couples nekRS to the MOOSE
heat conduction module.

## Geometry and Computational Model
  id=model

This section describes the [!ac](PB-FHR) reflector geometry and the simplifications
made in constructing a computational model of this system.
A top-down view of the [!ac](PB-FHR) reactor vessel is shown below. This geometry is
based on the specifications given in [!cite](shaver) and is summarized in
[table1].

The center region
is the pebble bed core, which is surrounded by two rings of graphite reflector blocks,
staggered with respect to one another in a brick-like fashion. The nominal
gap size between blocks is 0.002 m, but to ease meshing and make model depiction easier,
the gap between blocks is arbitrarily increased to 0.006 m.
Each ring contains 24 blocks. In black is shown the core barrel.

!media top_down.png
  id=top_down
  caption=Top-down schematic of the [!ac](PB-FHR) reactor core (only roughly to scale).
  style=width:60%;margin-left:auto;margin-right:auto

To form the entire axial height of the reflector, rings of blocks are stacked
vertically. The entire core height is 5.3175 m, or
about 10 vertical rings of blocks (additional structures at the core inlet and
outlet compose any remaining height not evenly divisible by 0.52 m).

!table id=table1 caption="Geometric and operating conditions relevant to reflector block modeling" style=width:60%
| Parameter | Value |
| :- | :- |
| Pebble core radius | 2.6 m |
| Inner block radial thickness | 0.3 m |
| Outer block radial thickness | 0.4 m |
| Barrel thickness | 0.022 m |
| Gap between blocks | 0.006 m |
| Gap between blocks and barrel | 0.02 m |
| Block height | 0.52 m |
| Core inlet temperature | 600&deg;C |
| Core outlet temperature | 700&deg;C |

In both the pebble bed region and through the small gaps between the reflector
blocks and between the reflector blocks and the barrel, FLiBe coolant flows. This bypass
flow path is shown as yellow dashed lines in [top_down]; in these gaps, coolant
flows both from the pebble bed and outward in the radial direction, as well as vertically
from lower reflector rings.
Each ring of blocks is separated from the rings above and below it by very thin horizontal
gaps that form as the graphite thermally expands (not shown in [top_down]; these flow paths also
contribute to bypass flows, allowing a second radial flow path from the pebble
bed towards the barrel. For simplicity, this second radial flow path is neglected here.

To reduce computational cost, the coupled nekRS-MOOSE simulation is conducted
for a single ring of reflector blocks (i.e. a height of 0.52 m), with azimuthal symmetry assumed to further
reduce the domain to half of an inner ring block, half of an outer ring block,
and the vertical and radial bypass flow paths between the blocks and the barrel. This
computational domain is shown outlined with a red dotted line in the figure above.
As already mentioned, the second radial flow path (along fluid "sheets" between vertically
stacked rings of blocks) is not considered. This geometry, as well
as the boundary conditions imposed, will be
described in much greater detail later when the meshes for the fluid and solid
phases are discussed.

## Non-Dimensional Formulation

nekRS can be solved in either dimensional or non-dimensional form; in the present case,
nekRS is solved in non-dimensional form. That is, characteristic scales for velocity,
temperature, length, and time are defined and substituted into the governing equations
so that all solution fields (velocity, pressure, temperature) are of order unity.
While the purpose of these tutorials is to be tutorials for Cardinal (i.e. not tutorials
for nekRS), some description of the non-dimensional formulation is important because it
is related to the data transfer between meshes.
A full derivation of the non-dimensional governing equations in nekRS is available
[here](https://nekrsdoc.readthedocs.io/en/latest/theory.html#non-dimensional-formulation).

Non-dimensional formulations for velocity, pressure, temperature, length,
and time are defined as

\begin{equation}
\label{eq:u_ref}
u_i^\dagger\equiv\frac{u_i}{U_{ref}}
\end{equation}

\begin{equation}
\label{eq:p_ref}
P^\dagger\equiv\frac{P}{\rho_0U_{ref}^2}
\end{equation}

\begin{equation}
\label{eq:T_ref}
T^\dagger\equiv\frac{T-T_{ref}}{\Delta T}
\end{equation}

\begin{equation}
\label{eq:x_ref}
x_i^\dagger\equiv\frac{x_i}{L_{ref}}
\end{equation}

\begin{equation}
\label{eq:t_ref}
t^\dagger\equiv\frac{t}{L_{ref}/U_{ref}}
\end{equation}

where a $\dagger$ superscript indicates a non-dimensional quantity and a "ref" subscript indicates
a characteristic scale. A "0" subscript indicates a reference parameter. Inserting these definitions into
the conservation of mass, momentum, and energy equations solved by nekRS gives

\begin{equation}
\label{eq:mass_nondim}
\frac{\partial u_i^\dagger}{\partial x_i^\dagger}=0
\end{equation}

\begin{equation}
\label{eq:momentum_nondim}
\rho^\dagger\left(\frac{\partial u_i^\dagger}{\partial t^\dagger}+u_j^\dagger\frac{\partial u_i^\dagger}{\partial x_j^\dagger}\right)=-\frac{\partial P^\dagger}{\partial x_i^\dagger}+\frac{1}{Re}\frac{\partial \tau_{ij}^\dagger}{\partial x_j^\dagger}+\rho^\dagger f_i^\dagger
\end{equation}

\begin{equation}
\label{eq:energy_nondim}
\rho^\dagger C_p^\dagger\left(\frac{\partial T^\dagger}{\partial t^\dagger}+u_i^\dagger\frac{\partial T^\dagger}{\partial x_i^\dagger}\right)=\frac{1}{Pe}\frac{\partial}{\partial x_i^\dagger}\left(k^\dagger\frac{\partial T^\dagger}{\partial x_i^\dagger}\right)+\dot{q}^\dagger
\end{equation}

New terms in these non-dimensional equations are $Re$ and $Pe$, the Reynolds and Peclet numbers,
respectively:

\begin{equation}
\label{eq:Re}
Re\equiv\frac{\rho_0 UL}{\mu_0}
\end{equation}

\begin{equation}
\label{eq:Pe}
Pe\equiv\frac{LU}{\alpha}
\end{equation}

nekRS solves for $\mathbf u^\dagger$, $P^\dagger$, and $T^\dagger$. Cardinal will handle
conversions from a non-dimensional solution to a dimensional MOOSE heat conduction application,
but awareness of this non-dimensional formulation will be important for describing the
nekRS input files.

In this model, the reference length scale is selected as the block gap width, or $L_{ref}=0.006$. Therefore,
to obtain a mesh with a gap width of unity (in non-dimensional units), the entire (dimensional) nekRS mesh must
be multiplied by $1/0.006$. The characteristic velocity is selected to obtain a Reynolds number of 100,
which corresponds to $U_{ref}=0.0575$ based on the values for density and viscosity for FLiBe
evaluated at 650&deg;C. A Reynolds number of 100 corresponds to a bypass fraction of about 7%, considering
that $Re$ can be written equivalent to [eq:Re] as

\begin{equation}
\label{eq:Re2}
Re\equiv\frac{\dot{m}L}{A\mu}
\end{equation}

where $\dot{m}$ is the mass flowrate (a fraction of the total core mass flowrate, distributed
among 48 half-reflector blocks) and $A$ is the inlet area. Further parametric studies for other
bypass fractions can be performed by varying the fluid Reynolds and Peclet numbers in the nekRS
input files.

Finally, $T_{ref}$ is taken equal to the inlet temperature of 923.15 K; the
$\Delta T$ parameter is arbitrary, and does not necessarily need to equal any expected temperature rise
in the fluid. So for this example, $\Delta T$ is simply taken equal to 10 K.

## Boundary Conditions

This section describes the boundary conditions imposed on the fluid and solid phases.
When the meshes are described in [#meshes], descriptive words such as "inlet" and "outlet"
will be directly tied to sidesets in the mesh to enhance the verbal description here.

### Solid Boundary Conditions

For the solid
domain, a fixed heat flux of 5 kW/m$^2$ is imposed on the block surface facing the pebble bed.
On the surface of the barrel, a heat convection boundary condition is imposed,

\begin{equation}
\label{eq:hfc}
q^{''}=h\left(T_s-T_\infty\right)
\end{equation}

where $q^{''}$ is the heat flux, $h$ is the convective heat transfer coefficient, and $T_\infty$
is the far-field ambient temperature.

Between the reflector blocks, the MOOSE heat conduction module is used to apply quadrature-based
radiation heat transfer across a transparent fluid.
For a paired set of boundaries, each quadrature point on boundary A is
paired with the nearest quadrature point on boundary B. Then, a radiation heat flux is imposed
between pairs of quadrature points as

\begin{equation}
\label{eq:hfr}
q^{''}=\sigma\frac{\left(T^4-T_{gap}^4\right)}{\frac{1}{\varepsilon_A}+\frac{1}{\varepsilon_B}-1}
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constant, $T$ is the temperature
at a quadrature point, $T_{gap}$ is the temperature of the nearest quadrature point across the
gap, and $\varepsilon_A$ and $\varepsilon_B$ are the emissivities of boundary A and B, respectively.

At fluid-solid interfaces, the solid temperature is imposed as a Dirichlet condition,
where nekRS computes the surface temperature.
Finally, the top and bottom of the block, as well as all symmetry boundaries, are treated
as insulated for simplicity.

### Fluid Boundary Conditions

At the inlet, the fluid temperature is taken as 650&deg;C, or the nominal
median fluid temperature. The inlet velocity is selected such that the Reynolds number is 100.
At the outlet, a zero pressure is imposed. On the $\theta=0^\circ$ boundary (i.e. the $y=0$ boundary), symmetry is imposed
such that all derivatives in the $y$ direction are zero. All other boundaries are treated as no-slip.

!alert note
The $\theta=7.5^\circ$ boundary (i.e. $360^\circ$ divided by 24 blocks, divided
in half because we are modeling only half a block) should also be imposed as a symmetry
boundary in the nekRS model. However, nekRS is currently limited to symmetry
boundaries only for boundaries aligned with the $x$, $y$, and $z$ coordinate
axes. Here, a no-slip boundary condition is used instead, so the correspondence of
the nekRS computational model to the depiction in [#model] is imperfect.

At fluid-solid interfaces, the heat flux is imposed as a Neumann condition, where MOOSE
computes the surface heat flux.

## Initial Conditions

Because the nekRS mesh contains very small elements in the fluid phase, fairly small time
steps are required to meet [!ac](CFL) conditions related to stability. Therefore,
the approach to the coupled, pseudo-steady conjugate heat transfer solution can be
accelerated by obtaining initial conditions from a pure conduction simulation. Then, the
initial conditions for the conjugate heat transfer simulation use the temperature obtained
from the conduction simulation, with a uniform axial velocity and zero pressure.
The process to run Cardinal in conduction mode is described in [#part1].

## Meshing
  id=meshes

This section describes how the meshes are generated for MOOSE and nekRS.
For both applications, the Cubit meshing software [!cite](cubit) is used to
programmatically create meshes with user-defined geometry and customizable
boundary layers. Journal files, or Python-scripted Cubit inputs, are used
to create meshes in Exodus II format. The MOOSE framework accepts meshes in
a wide range of formats that can be generated with many commercial and free
meshing tools; Cubit is used for this example because the content creator
is most familiar with this software, though similar meshes can be generated
with your preferred meshing tool.

!alert note
The meshes required for this example
are very large and are not versioned with the `tutorials` submodule in Cardinal -
please download them from [here](https://github.com/idaholab/virtual_test_bed/tree/main/pbfhr/reflector/meshes).

### Solid MOOSE Heat Conduction Mesh
  id=solid_mesh

The `solid.jou` file is a Cubit script that is used to generate the solid mesh,
shown below:

!listing /tutorials/fhr_reflector/meshes/solid.jou

The complete solid mesh (before a series of refinements) is shown below; the boundary names are illustrated towards
the right by showing only the highlighted surface to which each boundary corresponds.
Names are shown in Courier font. A unique block ID is used for the set of elements
corresponding to the inner ring, outer ring, and barrel. Material properties in MOOSE
are typically restricted by block, and setting three separate IDs allows us to set
different properties in each of these blocks.

!media solid_mesh.png
  id=solid_mesh
  caption=Solid mesh for the reflector blocks and barrel and a subset of the boundary names, before a series of mesh refinements

Unique boundary names are set for each boundary to which we will apply a unique
boundary condition; we define the boundaries on the top and bottom of the block,
the symmetry boundaries that reflect the fact that we've reduced the full [!ac](PB-FHR)
reflector to a half-block domain, and boundaries at the interface between the
reflector and the bed and on the barrel surface.

To facilitate radiation heat transfer between the thin block gaps,
additional boundaries must be defined on the faces on either side of the gaps.
Two boundaries are defined per gap - one on either side of the gap. These are
shown below, where the naming convention `three_to_two` indicates a boundary
on block 3, across a gap from block 2.

!media solid_mesh_radiation.png
  id=solid_radiation
  caption=Sidesets defined for enforcing radiation heat transfer boundary conditions
  style=width:50%;margin-left:auto;margin-right:auto

One convenient aspect of MOOSE is that the same elements
can be assigned to more than one boundary ID. To help in applying heat flux and
temperature boundary conditions between nekRS and MOOSE, we define another boundary
that contains all of the fluid-solid interfaces through which we will exchange
heat flux and temperature, as `fluid_solid_interface`. Some of the elements on
the `fluid_solid_interface` boundary are also present on the boundaries between
blocks used for the radiation boundary conditions shown in [solid_radiation].

### Fluid nekRS Mesh
  id=fluid_mesh

The `fluid.jou` is a Cubit script that is used to generate the fluid mesh, shown below.

!listing /tutorials/fhr_reflector/meshes/fluid.jou

The complete fluid mesh is shown below; the boundary names are illustrated towards
the right by showing only the highlighted surface to which each boundary corresponds.
Names are shown in Courier font. While the names of the surfaces are shown in Courier
font, nekRS does not directly use these names - rather, nekRS assigns boundary
conditions based on the numeric value of the boundary name; these are shown as "ID"
in the figure. An important restriction in nekRS is that the boundary IDs be ordered
sequentially beginning from 1.

!media fluid_mesh.png
  id=fluid_mesh
  caption=Fluid mesh for the FLiBe flowing around the reflector blocks, along with boundary names and IDs. It is difficult to see, but the `porous_inner_surface` boundary corresponds to the thin surface at the interface between the reflector region and the pebble bed.

A strength of Cardinal for conjugate heat transfer applications
is that the fluid and solid meshes do not need to share nodes on a common surface; libMesh
mesh-to-mesh data interpolations apply to surfaces of very different refinement and position in
space; meshes may even overlap, such as for curvilinear applications. [zoom_mesh] shows
a zoom-in of the two mesh files (for the fluid and solid phases); rather than being limited
with a continuous mesh mapping from the fluid phase inwards to the solid phase, each phase
can be meshed according to its physics requirements.

!media zoom_mesh.png
  id=zoom_mesh
  caption=Zoomed-in view of the fluid and solid meshes, overlaid in Paraview. Lines are element boundaries.
  style=width:50%;margin-left:auto;margin-right:auto

Because nekRS uses a custom binary mesh format with a `.re2` extension, a conversion utility
must be used to convert an Exodus II format to nekRS's `.re2` nekRS format. This utility, or the `exo2nek` program,
requires the Exodus mesh elements to be type `HEX20` (a twenty-node hexahedral element),
so we set this element type from Cubit. Instructions for building
the `exo2nek` program are available [here](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html).
After this script has been compiled, you simply need to run

```
$ exo2nek
```

and then follow the prompts for the information that must be added about the mesh
to perform the conversion. Above, `$` indicates the terminal prompt.
For this example, there is no solid mesh, there are no periodic surface pairs,
and we want a scaling factor of $1/0.006$ (for the non-dimensional formulation)
and no translation. The last part of the `exo2nek` program will request
a name for the fluid `.re2` mesh; providing `fluid` then will create the nekRS
mesh, named `fluid.re2`.

## Part 1: Initial Conduction Coupling
  id=part1

In this section, nekRS and MOOSE are coupled for conduction heat transfer in the solid reflector
blocks and barrel, and through a stagnant fluid. The purpose of this stage of
the analysis is to obtain a restart file for use as an initial condition in [#part2] to accelerate the nekRS
calculation for conjugate heat transfer, since the energy equation is slowest to converge.
Because this initial condition is only used for
accelerating a later calculation, applying the radiation quadrature-based boundary conditions
is deferred to [#part2].

All input files for this stage of the analysis are present in the
`tutorials/fhr_reflector/conduction` directory. The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and are described in the `solid.i` input.
At the top of this file, the core heat flux is defined as a variable local to the file.

!listing /tutorials/fhr_reflector/conduction/solid.i
  end=Mesh

The value of this variable can then be used anywhere else in the input file
with syntax like `${core_heat_flux}`, similar to bash syntax. Next, the solid mesh is
specified by pointing to the Exodus mesh generated in [#solid_mesh].

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Mesh
  end=Variables

The heat conduction module will solve for temperature, which is defined as a nonlinear
variable.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Variables
  end=AuxVariables

The [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
 system in MOOSE is used to communicate auxiliary variables across applications;
a boundary heat flux will be computed by MOOSE and applied as a boundary condition in nekRS.
In the opposite direction, nekRS will compute a surface temperature that will be applied as
a boundary condition in MOOSE. Therefore, both the flux (`flux`) and surface temperature
(`nek_temp`) are declared as auxiliary variables. The solid app will *compute* `flux`,
while `nek_temp` will simply *receive* a solution from the nekRS sub-application. The flux
is computed as a constant monomial field (a single value per element) due to the manner in
which material properties are accessible in auxiliary kernels in MOOSE.

!listing /pbfhr/reflector/conduction/solid.i
  start=AuxVariables
  end=Functions

