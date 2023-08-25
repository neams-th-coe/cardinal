# Solid and Fluid Coupling of OpenMC and MOOSE

In this tutorial, you will learn how to:

- Couple OpenMC to 2 separate MOOSE applications solving for the solid and fluid thermal-fluids
- Couple OpenMC to mixed-dimension feedback with 3-D heat conduction and 1-D fluid flow
- Establish coupling between OpenMC and MOOSE for nested universe OpenMC models
- Apply homogenized temperature feedback to heterogeneous OpenMC cells

To access this tutorial:

```
cd cardinal/tutorials/gas_assembly
```

This tutorial also requires you to download
an OpenMC XML file from Box. Please download the files from the
`gas_assembly` folder [here](https://anl.app.box.com/folder/141527707499?s=irryqrx97n5vi4jmct1e3roqgmhzic89)
and place these files within the same directory structure
in `tutorials/gas_assembly`.

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
For testing purposes, you may choose to decrease the number of particles to
solve faster, or decrease the resolution of the solid mesh.
!alert-end!

In this tutorial, we couple OpenMC to the MOOSE heat conduction module
and the [Thermal Hydraulics Module (THM)](https://mooseframework.inl.gov/modules/thermal_hydraulics/index.html)
, a set of 1-D systems-level thermal-hydraulics kernels.
OpenMC will receive temperature feedback from both
the MOOSE heat conduction module (for the solid regions) and from THM
(for the fluid regions). Density feedback will be provided by THM
for the fluid regions.
This tutorial models a full-height [!ac](TRISO)-fueled prismatic gas reactor
fuel assembly, and is
an extension of [an earlier tutorial](gas_compact.md) which coupled
OpenMC and MOOSE heat conduction (i.e. without an application providing fluid
feedback) for a unit cell version of the same geometry. In this
tutorial, we add fluid feedback and also describe several nuances associated with
setting up feedback in OpenMC lattices.

This tutorial was developed with support from the NEAMS Thermal Fluids Center
of Excellence. A paper [!cite](novak_2021c)
describing the physics models and mesh refinement studies provides additional
context beyond the scope of this tutorial.

Due to the tall height of the full
assembly (about 6 meters), the converged results shown in [!cite](novak_2021c)
and in the figures in this tutorial used a finer mesh and more particles
than in the input files
we set up for this tutorial - the input files in this tutorial still require
parallel resources to run, but should be faster-running.
Parameters that have been set to coarser values are noted where applicable.

## Geometry and Computational Model

The geometry consists of a [!ac](TRISO)-fueled gas reactor assembly [!cite](sterbentz).
A top-down view of the geometry is shown in [assembly].
The assembly is a graphite prismatic hexagonal block with 108 helium coolant channels,
210 fuel compacts, and 6 poison compacts. Each fuel compact contains [!ac](TRISO)
particles dispersed in a graphite matrix with a packing fraction of 15%.
All channels and compacts are ordered in a
triangular lattice with pitch $p_{cf}$.
Due to irradiation- and temperature-induced swelling of graphite, small helium gaps
exist between assemblies. In this tutorial, rather than explicitly model the inter-assembly
flow, we treat the gap regions as solid graphite.
There are also graphite reflectors above and below the assembly.

!media assembly.png
  id=assembly
  caption=[!ac](TRISO)-fueled gas reactor fuel assembly
  style=width:80%;margin-left:auto;margin-right:auto

The [!ac](TRISO) particles use a conventional design that consists of a central
fissile uranium oxycarbide kernel enclosed in a carbon buffer, an inner
[!ac](PyC) layer, a silicon carbide layer, and finally an outer
[!ac](PyC) layer. The geometric specifications for the assembly
dimensions are shown in [assembly], while dimensions for the
[!ac](TRISO) particles are summarized in [table1].

!table id=table1 caption=Geometric specifications for the [!ac](TRISO) particles
| Parameter | Value (cm) |
| :- | :- |
| TRISO kernel radius | 214.85e-4 |
| Buffer layer radius | 314.85e-4 |
| Inner PyC layer radius | 354.85e-4 |
| Silicon carbide layer radius | 389.85e-4 |
| Outer PyC layer radius | 429.85e-4 |

Heat is produced in the [!ac](TRISO) particles to yield a total power of
16.7 MWth. This heat is removed by helium flowing downwards through the coolant
channels with a total mass flowrate of 9.775 kg/s, which is assumed to be uniformly
distributed among the coolant channels. The outlet pressure is 7.1 MPa.

### Heat Conduction Model

!include steady_hc.md

To greatly reduce meshing requirements, the [!ac](TRISO) particles
are homogenized into the compact regions by volume-averaging material properties.
The solid mesh is shown in [solid_mesh]. The only sideset in the domain
is the coolant channel surface, which is named `fluid_solid_interface`.
The solid geometry uses a length unit of meters.
The solid mesh is created using the MOOSE reactor module [!cite](shemon_2021),
which provides easy-to-use
mesh generators to programmatically construct reactor core meshes as building blocks of bundle
and pincell meshes.

!media assembly_solid_mesh.png
  id=solid_mesh
  caption=Converged mesh for the solid heat conduction model (with 300 axial layers); to make the tutorial run faster, our mesh will only use 100 axial layers
  style=width:80%;margin-left:auto;margin-right:auto

The file used to generate the solid mesh is shown below. The mesh is created
by first building pincell meshes for a fuel pin, a coolant pin, a poison pin,
and a graphite "pin" (to represent the central graphite region). The pin
meshes are then combined together into a bundle pattern and extruded.

!listing /tutorials/gas_assembly/solid_mesh.i
  end=Problem

You can create this mesh by running:

```
cardinal-opt -i common_input.i solid_mesh.i --mesh-only
```

which will create a mesh named `solid_mesh_in.e`.
Note that the above command
takes advantage of a MOOSE feature for combining input files together by placing
some common parameters used by the other applications into a file named `common_input.i`.

The temperature on the fluid-solid interface is provided by [!ac](THM),
while the heat source is provided by OpenMC.
Because MOOSE heat conduction will run first in the coupled case,
the initial fluid temperature is
set to an axial distribution given by bulk energy conservation ($q=\dot{m}C_{p,f}\left(T_f-T_{inlet}\right)$)
given the inlet temperature $T_{inlet}$, mass flowrate $\dot{m}$, and fluid
isobaric specific heat $C_{p,f}$. The initial heat source distribution is assumed
uniform in the radial direction with a sinusoidal dependence in the axial direction.

### OpenMC Model

The OpenMC model is built using [!ac](CSG). The [!ac](TRISO) positions are sampled
using the [!ac](RSA) [algorithm in OpenMC](https://docs.openmc.org/en/stable/examples/triso.html).
OpenMC's Python [!ac](API) is
used to create the model with the script shown below. First, we define materials
for the various regions. Next, we create a single [!ac](TRISO) particle universe
consisting of the five layers of the particle and an infinite extent of graphite
filling all other space. We then pack uniform-radius spheres into a cylindrical
region representing a fuel compact, setting each sphere to be filled with the
[!ac](TRISO) universe.

Next, we loop over 50 axial layers and create a unique hexagonal lattice for each layer.
This hexagonal lattice defines the fuel assembly structure, and consists of four different
universes:

- A fuel pin plus surronding matrix (`f`),
- A coolant channel plus surrounding matrix (`c`),
- A boron carbide poision pin plus surrounding matrix (`p`), and
- A homogeneous graphite hexagonal pincell to fill
  the "boundaries" and centermost region (`g`).

In each layer we set up the lattice
structure by listing the universes in each "ring" of the lattice, with `ring0` being
the centermost ring and `ring11` being the outermost ring.

Recall that temperatures in OpenMC can be set directly on the cell, but that fluid densities
can only be set on *materials*. For this reason, we need to create 108 unique coolant materials
for each axial plane if we want to be able to set unique densities in each coolant channel
region. Rather than creating 108 materials in a loop,
we use the `clone()` feature in OpenMC to clone an existing coolant material 108 times per layer.
This duplicates the material properties (densities and isotopic composition), but assigns
a new ID that allows individual tracking of density. The Python script used to create the
OpenMC model is shown below.

!listing /tutorials/gas_assembly/assembly.py language=python

The level on which we will apply feedback from MOOSE is 1, because the geometry
consists of a hexagonal lattice (level 0), and we want to apply individual cell feedback
within that lattice (level 1). For the solid phase, this selection is equivalent to
applying a single temperature (per compact and per layer) for a compact region -
all [!ac](TRISO) particles and the surrounding matrix in each compact receives a uniform
temperature. Finally, to accelerate the particle tracking, we:

- Repeat the same [!ac](TRISO) universe in each axial layer and within each compact
- Superimpose a Cartesian search lattice in the fuel channel regions.

The OpenMC geometry, colored by either cell ID or instance, is shown in
[openmc_model]. Not shown are the axial reflectors on the top
and bottom of the assembly. The lateral faces are periodic, while the top
and bottom boundaries are vacuum.
The Cartesian search lattice in the fuel compact
regions is also visible in [openmc_model].

!media assembly_cells.png
  id=openmc_model
  caption=OpenMC model, colored by cell ID or instance
  style=width:80%;margin-left:auto;margin-right:auto

Cardinal applies uniform temperature and density feedback to OpenMC
for each unique cell ID $+$ instance combination. For this setup,
OpenMC receives on each axial plane a total of 721 temperatures and 108 densities
(one density per coolant channel). With references to the colors shown in
[openmc_model], the 721 cell temperatures correspond to:

\begin{equation*}
210\underbrace{\text{ fuel compacts}}_{\substack{\textup{1 TRISO compact (\textit{rainbow})}\\\textup{1 matrix region (\textit{purple})}}}\ \ +\ \ \ 108\underbrace{\text{ coolant channels}}_{\substack{\textup{1 coolant region (\textit{various})}\\\textup{1 matrix region (\textit{various})}}}\ \ +\ \ \ 6\underbrace{\text{ poison compacts}}_{\substack{\textup{1 poison region (\textit{brown})}\\\textup{1 matrix region (\textit{blue})}}}\ \ +\ \ \ 73\underbrace{\text{ graphite fillers}}_\text{1 matrix region (\textit{mustard})}
\end{equation*}

The solid temperature is provided by the MOOSE heat conduction module,
while the fluid temperature and density are provided by [!ac](THM).
Because we will run OpenMC second, the initial fluid temperature is
set to the same initial condition that is imposed in the MOOSE solid model.
The fluid density is then set using the ideal gas [!ac](EOS) at a fixed pressure
of 7.1 MPa given the imposed temperature, i.e. $\rho_f(P,T_f)$.

To create the XML files required to run OpenMC, run the script:

```
python assembly.py
```

You can also use the XML files checked in to the `tutorials/gas_assembly` directory;
if you use these already-existing files, you will also need to download the `geometry.xml` file
from Box; this file is large due to the saved [!ac](TRISO) geometry information.

### THM Model

!include thm_eqns.md

In this tutorial, we use 50 elements for each channel.
The mesh is constructed automatically within [!ac](THM).
To simplify the specification of
material properties, the fluid geometry uses a length unit of meters.
The heat flux imposed in the [!ac](THM) elements is obtained by area averaging the heat flux from
the heat conduction model in 50 layers along the fluid-solid interface. For the reverse transfer, the wall temperature
sent to MOOSE heat conduction is set to a uniform value along the
fluid-solid interface according to a nearest-node mapping to the [!ac](THM) elements.

Because [!ac](THM) will run last in the coupled case, initial conditions are only required for pressure,
fluid temperature, and velocity, which are set to uniform distributions.

## Multiphysics Coupling

A summary of the data transfers between the three applications is shown in
[data_transfers]. The inset describes the 1-D/3-D data transfers with [!ac](THM).

!media assembly_coupling.png
  id=data_transfers
  caption=Summary of data transfers between OpenMC, MOOSE, and [!ac](THM)
  style=width:80%;margin-left:auto;margin-right:auto

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described
in the `solid.i` input. We define a number of constants at the beginning of the file
and set up the mesh from a file.

!listing /tutorials/gas_assembly/solid.i
  end=Variables

Next, we define the temperature variable, `T`, and specify the governing equations
and boundary conditions we will apply.

!listing /tutorials/gas_assembly/solid.i
  start=Variables
  end=Functions

The MOOSE heat conduction module will receive power from OpenMC in the form of an
[AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html),
so we define a receiver variable for the fission power, as `power`. The MOOSE heat
conduction module will also receive a fluid wall temperature from [!ac](THM)
as another [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)
which we name `thm_temp`. Finally, the MOOSE heat conduction module will send the heat
flux to [!ac](THM), so we add a variable named `flux` that we will use to compute
the heat flux using the [DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html)
auxiliary kernel.

!listing /tutorials/gas_assembly/solid.i
  start=AuxVariables
  end=Executioner

We use functions to define the thermal conductivities. The material
properties for the [!ac](TRISO) compacts are taken as volume averages of the various constituent
materials. We will evaluate the thermal conductivity for the boron carbide as a
function of temperature by using `t` (which *usually* is interpeted as time) as
a variable to represent temperature. This is syntax supported
by the [HeatConductionMaterials](https://mooseframework.inl.gov/source/materials/HeatConductionMaterial.html)
used to apply these functions to the thermal conductivity.

!listing /tutorials/gas_assembly/solid.i
  start=Functions
  end=Postprocessors

We define a number of postprocessors for querying the solution as well as for
normalizing the fission power and heat flux, to be described at greater
length in [#n1].

!listing /tutorials/gas_assembly/solid.i
  block=Postprocessors

For visualization purposes only, we add
[LayeredAverages](https://mooseframework.inl.gov/source/userobject/LayeredAverage.html)
for the fuel and block temperatures. These will average the temperature in layers
oriented in the $z$ direction, which we will use for plotting axial temperature
distributions. We output the results of these userobjects to CSV using
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html) and by setting `csv = true` in the output.

!listing /tutorials/gas_assembly/solid.i
  start=UserObjects

Finally, we specify a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner. Because there are no time-dependent kernels in this input file,
this is equivalent in practice to using a [Steady](https://mooseframework.inl.gov/source/executioners/Steady.html)
executioner, but allows you to potentially sub-cycle the MOOSE heat conduction solve
relative to the OpenMC solve
(such as if you wanted to converge the
[!ac](CHT) fully inbetween data exchanges with OpenMC).

!listing /tutorials/gas_assembly/solid.i
  block=Executioner

### Fluid Input Files

The fluid phase is solved with [!ac](THM), and is described in the `thm.i` input.
This input file is built using syntax specific to [!ac](THM) - we will only briefly
cover this syntax, and instead refer users to the [THM documentation](https://mooseframework.inl.gov/modules/thermal_hydraulics/index.html)
 for more information.
First we define a number of constants at the beginning of the file and apply
some global settings. We set the initial conditions for pressure, velocity,
and temperature and indicate the fluid [!ac](EOS) object
using [IdealGasFluidProperties](https://mooseframework.inl.gov/source/userobjects/IdealGasFluidProperties.html).

!listing /tutorials/gas_assembly/thm.i
  end=AuxVariables

Next, we define the "components" in the domain. These components essentially consist
of the physics equations and boundary conditions solved by [!ac](THM), but expressed
in [!ac](THM)-specific syntax. These components define
single-phase flow in a pipe, an inlet mass flowrate boundary condition, an outlet
pressure boundary condition, and heat transfer to the pipe wall.

!listing /tutorials/gas_assembly/thm.i
  block=Components

Associated with these components are a number of closures, defined as materials.
We set up the Churchill correlation for the friction factor and the Dittus-Boelter
correlation for the convective heat transfer coefficient. Additional materials are
created to represent dimensionless numbers and other auxiliary terms, such as the
wall temperature. As can be seen here,
the [Material](https://mooseframework.inl.gov/syntax/Materials/index.html) system
is not always used to represent quantities traditionally thought of as "material properties."

!listing /tutorials/gas_assembly/thm.i
  block=Materials

[!ac](THM) computes the wall temperature to apply a boundary condition in
the MOOSE heat conduction module. To convert the `T_wall` material into an auxiliary
variable, we use the [ADMaterialRealAux](https://mooseframework.inl.gov/source/auxkernels/MaterialRealAux.html).

!listing /tutorials/gas_assembly/thm.i
  start=AuxVariables
  end=Materials

Finally, we set the preconditioner, a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner,
and set an Exodus output. The `steady_state_detection` and `steady_state_tolerance`
parameters will automatically terminate the [!ac](THM) solution once the relative
change in the solution is less than $10^{-8}$.

!listing /tutorials/gas_assembly/thm.i
  start=Preconditioning

As you may notice, this [!ac](THM) input file only models a single coolant flow
channel. We will leverage a feature in MOOSE that allows a single application to be
repeated multiple times throughout a main application without having to
merge input files or perform other transformations. We will run OpenMC as
the main application; the syntax needed to achieve this setup is covered next.

### Neutronics Input Files
  id=n1

The neutronics physics is solved with OpenMC over the entire domain. The
OpenMC wrapping is described in the `openmc.i` file. We begin by defining
a number of constants and by setting up the mesh mirror on which OpenMC
will receive temperature and density from the coupled applications, and on which
OpenMC will write the fission heat source. Because the coupled MOOSE applications
use length units of meters, the mesh mirror must also be in units of meters in order
to obtain correct data transfers. For simplicity, we use the same solid mesh as
used by the solid heat conduction solution, but this is not required.
For the fluid region, we use MOOSE
mesh generators to construct a mesh for a single coolant channel, and then
repeat it for the 108 coolant channels.

!listing /tutorials/gas_assembly/openmc.i
  end=AuxVariables

Before progressing further, we first need to describe the multiapp structure
connecting OpenMC, MOOSE heat conduction, and [!ac](THM). We set the main application
to OpenMC, and will have both MOOSE heat conduction and [!ac](THM) as
"sibling" sub-applications.

!alert note
At the time of writing, the MOOSE framework
does not support "sibling" multiapp transfers, meaning that the data to be
communicated between MOOSE heat conduction and [!ac](THM)
(the heat flux and wall temperature data transfers in [data_transfers]) must go "up a level"
to their common main application. This has since been relaxed.

Therefore, we will see in the OpenMC
input file information related to data transfers between MOOSE heat
conduction and [!ac](THM). The auxiliary variables defined for the OpenMC
model are shown below.

!listing /tutorials/gas_assembly/openmc.i
  start=AuxVariables
  end=ICs

For visualization purposes,
we use a [CellTemperatureAux](/auxkernels/CellTemperatureAux.md) to view
the temperature set in each OpenMC cell and a [CellDensityAux](/auxkernels/CellDensityAux.md)
to view the density set in each fluid OpenMC cell. To understand how the OpenMC
model maps to the `[Mesh]`, we also include
[CellMaterialIDAux](/auxkernels/CellMaterialIDAux.md),
[CellIDAux](/auxkernels/CellIDAux.md), and
[CellInstanceAux](/auxkernels/CellInstanceAux.md) auxiliary kernels.
Next, we add a receiver
`flux` variable that will hold the heat flux received from MOOSE (and sent
to [!ac](THM)) and another receiver variable `thm_temp_wall` that will hold
the wall temperature received from [!ac](THM) (and sent to MOOSE).

Finally, to reduce the number
of transfers from [!ac](THM), we will receive fluid temperature from
[!ac](THM), but re-compute the density locally in the OpenMC wrapping
using a [FluidDensityAux](https://mooseframework.inl.gov/source/auxkernels/FluidDensityAux.html)
with the same [!ac](EOS) as used in the [!ac](THM) input files.

!listing /tutorials/gas_assembly/openmc.i
  block=FluidProperties

Next, we set initial conditions for the fluid wall temperature, the fluid bulk
temperature, and the heat source. We set these initial conditions in the OpenMC
wrapper because the very first time that the transfers to the MOOSE heat
conduction module and to [!ac](THM) occur, these initial conditions will be passed.

!listing /tutorials/gas_assembly/openmc.i
  start=ICs
  end=FluidProperties

The `[Problem]` block is then used to specify settings for the OpenMC wrapping. We
define the total power for normalization, indicate that blocks 1, 2, and 4 are solid
(graphite, compacts, and poison) while block 101 is fluid. We automatically add
tallies to block 2, the fuel compacts. Because OpenMC solves in units of centimeters,
we specify a `scaling` of 100, i.e. a multiplicative factor to apply to the
`[Mesh]` to get into OpenMC's centimeter units.

!listing /tutorials/gas_assembly/openmc.i
  block=Problem

Other features we use include an output of the fission tally standard deviation
in units of W/m$^3$ to the `[Mesh]` by setting `output = 'unrelaxed_tally_std_dev'`.
This is used to obtain uncertainty estimates of the heat source distribution from OpenMC
in the same units as the heat source. We also leverage a helper utility
in Cardinal by setting `check_equal_mapped_tally_volumes = true`. This parameter will
throw an error if the tallied OpenMC cells map to different volumes in the MOOSE domain.
Because we know *a priori* that the equal-volume OpenMC tally cells *should* all map
to equal volumes, this will help ensure that the volumes used for heat source normalization
are also all equal. For further discussion of this setting and a pictorial description
of the possible effect of non-equal mapped vlumes, please see the
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) documentation.

We also set `identical_cell_fills` to the set of subdomains for which OpenMC's
cells have identical fills. This is an optimization that greatly
reduces the initialization time for large [!ac](TRISO) problems. During setup of an
OpenMC wrapping, we need to cache all the cells contained within the [!ac](TRISO) compacts
so that we know all the contained cells to set the temperatures for. This process can
be quite time-consuming if the search needs to be repeated for every single [!ac](TRISO)
compact cell (210 compacts times 50 axial layers = 10,500 contained cell searches).
The `identical_cell_fills` option is used to indicate whether your problem can
leverage a speedup that applies to models where *every lattice/universe-filled tally cell*
has *exactly* the same filling lattice/universe. In other words, we set up our problem
to use the same [!ac](TRISO) universe in each layer of each fuel compact. This means that
the cells filling each [!ac](TRISO) compact can be deduced by following a pattern based
on the first two fuel compacts, letting us omit 10,498 of the contained cell searches.

!alert note
When first using this optimization for a new problem, we recommend setting
`check_identical_cell_fills = true` so that you can do an exact comparison
against the "rigorous" approach to be sure that your problem setup has the necessary
prerequisites to use this feature. After you verify that no errors are thrown during
setup, set `check_identical_cell_fills` to `false` (the default)
to use this initialization speedup feature.

Because the blocks in the OpenMC mesh mirror
receive temperatures from different applications, we use the
`temperature_variables` and `temperature_blocks` parameters of
[OpenMCCellAverageProblem](https://cardinal.cels.anl.gov/source/problems/OpenMCCellAverageProblem.html)
to automatically create separate variables for OpenMC to read temperature from
in different parts of the domain.
The `temperature_blocks` and
`temperature_variables` parameters allow you to customize exactly the variable names from
which to read temperature.

Finally, we apply a constant relaxation model to the heat source. A constant
relaxation will compute the heat source in iteration $i+1$ as an average
of the heat source from iteration $i$ and the most-recently-computed heat source,
indicated here as a generic operator $\Phi$ which represents the Monte Carlo solve:

\begin{equation}
\label{eq:relax}
q^{i+1}=\left(1-\alpha\right)q^i+\alpha\Phi
\end{equation}

In [eq:relax], $\alpha$ is the relaxation factor, which we set here to 0.5. In other
words, the heat source in iteration $i+1$ is an average of the most recent Monte
Carlo solution and the previous iterate. This is necessary to accelerate the fixed
point iterations and achieve convergence in a reasonable time - otherwise oscillations
can occur in the coupled physics.

We run OpenMC as the main application, so we next need to define
[MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html) to run
the solid heat conduction model and the [!ac](THM) fluid model as the sub-applications.
We also require a number of transfers both for 1) sending necessary coupling data between
the three applications and 2) visualizing the combined [!ac](THM) output. To couple OpenMC
to MOOSE heat conduction, we use four transfers:

- [MultiAppShapeEvaluationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppShapeEvaluationTransfer.html)
  to transfer:

  - power from OpenMC to MOOSE (with conservation of total power)

- [MultiAppGeometricInterpolationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeometricInterpolationTransfer.html)
  to transfer:

  - solid temperature from MOOSE to OpenMC
  - wall temperature from OpenMC (which doesn't directly compute the wall temperature, but
    instead receives it from [!ac](THM) through a separate transfer) to MOOSE

- [MultiAppGeneralFieldNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestNodeTransfer.html)
  to transfer and conserve heat flux from MOOSE to OpenMC (which isn't used directly in OpenMC, but instead
  gets sent later to [!ac](THM) through a separate transfer)

To couple OpenMC to [!ac](THM), we require three transfers:

- [MultiAppGeneralFieldUserObjectTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldUserObjectTransfer.html)
  to send the layer-averaged wall heat flux from OpenMC (which computes the layered-average heat flux from the heat
  flux received from MOOSE heat conduction) to [!ac](THM)
- [MultiAppGeneralFieldNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestNodeTransfer.html)
  to transfer:

  - fluid wall temperature from [!ac](THM) to OpenMC (which isn't used directly in OpenMC, but instead
    gets sent to MOOSE heat conduction in a separate transfer)
  - fluid bulk temperature from [!ac](THM) to OpenMC

For visualization purposes, we also send the pressure and velocity computed by
[!ac](THM) to the OpenMC mesh mirror.

!listing /tutorials/gas_assembly/openmc.i
  start=MultiApps
  end=UserObjects

To compute the layer-averaged heat flux on the surface of each coolant channel
(which is used as a boundary condition in [!ac](THM)), we use a
[NearestPointLayeredSideAverage](https://mooseframework.inl.gov/source/userobject/NearestPointLayeredSideAverage.html)
user object, where by providing the center points of each of the coolant channels,
we can get a unique heat flux along each channel wall.  We also add several
[LayeredAverage](https://mooseframework.inl.gov/source/userobject/NearestPointLayeredAverage.html)
user objects in order to compute radially-averaged power, temperatures,
pressures, and velocities that we will use later in making axial plots
of the solution. We can automatically output these user objects into
CSV format by translating the user objects into
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html).
A number of postprocessors are included related to the Monte Carlo
solution, as well as inlet pressure and pressure drop. See the next section
for further description.

!listing /tutorials/gas_assembly/openmc.i
  start=UserObjects
  end=Executioner

Finally, we use a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner and specify Exodus and CSV output formats. Note that the time step size is
inconsequential in this case, but instead represents the Picard iteration. We will run for 10
"time steps," which represent fixed point iterations.

!listing /tutorials/gas_assembly/openmc.i
  start=Executioner

## Execution and Postprocessing
  id=results

To run the coupled calculation,

```
mpiexec -np 4 cardinal-opt -i common_input.i openmc.i --n-threads=2
```

This will run with 4 MPI processes and 2 OpenMP threads per rank.
This tutorial uses quite large meshes due to the
6 meter height of the domain - if you wish to run this tutorial with fewer computational
resources, you can reduce the height and the various mesh parameters (number of extruded
layers and elements in the [!ac](THM) domain) and then recreate the OpenMC model and meshes.
Recall that all results shown in this section correspond to the same input files
but with 300 extrusion layers in the solid, 150 [!ac](THM) elements per channel,
2000 particles per batch, 500 inactive batches, and 2000 active batches - to get a faster-running tutorial, we reduced all
of these parameters to coarser values.

When the simulation has completed, you will have created a number of different output files:

- `openmc_out.e`, an Exodus file with the OpenMC solution and the data that was
  ultimately transferred in/out of OpenMC
- `openmc_out_bison0.e`, an Exodus file with the solid solution
- `openmc_out_thm<n>.e`, Exodus files with each of the `<n>` [!ac](THM) solutions
- `openmc_out.csv`, a CSV file with the results of the postprocessors in the
  OpenMC wrapping input file for each time step
- `openmc_out_bison0.csv`, a CSV file with the results of the postprocessors
  in the solid heat conduction input file for each time step
- `openmc_out_bison0_block_axial_avg_<n>`.csv, a CSV file with the layer-averaged
  block temperature at time step `<n>`
- `openmc_out_bison0_flux_axial_avg_<n>`.csv, a CSV file with the layer-averaged
  fluid-solid interface heat flux at time step `<n>`
- `openmc_out_bison0_fuel_axial_avg_<n>`.csv, a CSV file with the layer-averaged
  compact temperature at time step `<n>`
- `openmc_out_fluid_avg_<n>.csv`, a CSV file with the layer-averaged fluid bulk
  temperature at time step `<n>`
- `openmc_out_power_avg_<n>`.csv, a CSV file with the layer-averaged heat source
  at time step `<n>`
- `openmc_out_pressure_avg_<n>`.csv, a CSV file with the layer-averaged pressure
  at time step `<n>`
- `openmc_out_velocity_avg_<n>.csv`, a CSV file with the layer-average axial fluid
  velocity at time step `<n>`

Coupled convergence is defined at the point when 1) the $k$ eigenvalue
is within the uncertainty band of the previous iteration and 2)
there is less than 2 K absolute change in maximum fuel, block, and fluid temperatures.
Convergence is obtained in 6 fixed point iterations. [convergence] shows the
maximum fuel, compact, and fluid temperatures, along with the $k$ eigenvalue,
as a function of iteration number for all 10 iterations run. For each iteration, we first run MOOSE
heat conduction (which we indicate as iterations A1, A2, and so on), followed
by OpenMC (which we indicate as iterations B1, B2, and so on), and finally
[!ac](THM) (which we indicate as iterations C1, C2, and so on). In other words,
in iteration $i$:

- Iteration A$i$ represents a MOOSE heat conduction solve
  using the power and fluid-solid wall temperature from iteration $i-1$
- Iteration B$i$ represents an OpenMC solve using the solid temperature
  from iteration $i$ and the fluid temperature and density from iteration $i-1$
- Iteration C$i$ represents a [!ac](THM) solve using the fluid-solid wall heat
  flux from iteration $i$

!media assembly_convergence.png
  id=convergence
  caption=Convergence in various metrics as a function of iteration number

[power_convergence] shows the radially-averaged power from OpenMC as a function
of iteration number. There is essentially no change in the axial distribution beyond
6 fixed point iterations, which further confirms that we have obtained a converged solution.
The remainder of the depicted results correpond to iteration 6.

!media assembly_q_iteration.png
  id=power_convergence
  caption=Radially-average fission power distribution as a function of iteration number $i$
  style=width:40%;margin-left:auto;margin-right:auto

[power] shows the fission power distribution
computed by OpenMC. The inset shows the power distribution on an $x-y$ plane 2.15 meters from the inlet,
where the maximum power occurs. Slight azimuthal asymmetries exist due to the finite tally
uncertainty.
Neutrons reflecting back into the fuel region from the axial reflectors
cause local power peaking at the ends of the assembly, while the negative fuel temperature
coefficient causes the power distribution to shift towards the reactor inlet
where temperatures are lower.
The six poison compacts in the corners result in local power depression such that
the highest compact powers occur near the center of the assembly.

!media assembly_power.png
  id=power
  caption=Fission power predicted by OpenMC; note the use of a separate color scale for the inset.

[assembly_temp] shows the solid temperature (left)
computed by MOOSE and the cell temperature imposed in OpenMC (right).
The bottom row shows the temperature on an $x-y$ plane 2.15 meters from the inlet.
The insulated boundary conditions, combined with a lower
"density" of coolant channels near the lateral faces, result in higher compact temperatures
near the assembly peripheries, except in the vicinity of the poison pins where the increased
absorption reduces the local power densities.
Each OpenMC cell is set to the volume-average temperature from the mesh mirror elements
whose centroids mapped to each cell; a similar procedure is used to set
cell temperatures and densities for the fluid cells.

!media assembly_temp.png
  id=assembly_temp
  caption=Solid temperature predicted by MOOSE (left) and volume-average temperature imposed in OpenMC (right). Note the use of a separate color scale for the insets.

[fluid_temp] shows the solid temperature computed by MOOSE on several $x-y$ planes
with the fluid temperature computed by [!ac](THM) shown as tubes. An inset shows the fluid temperature
on the outlet plane. The absence of compacts in the center region results in the lowest
fluid temperatures in this region, while the highest fluid temperatures are observed for channels surrounded by 6 compacts
that are sufficiently close to the periphery to be affected by the lateral insulated boundary conditions.

!media assembly_fluid_temp.png
  id=fluid_temp
  caption=Fluid temperature predicted by [!ac](THM) (tubes and inset) and solid temperature predicted by MOOSE (five slices). Note the use of three separate color scales.

Finally, [assembly_averages] shows the radially-averaged fission distribution and fluid, compact, and graphite temperatures (left);
and velocity and pressure (right) as a function of axial position. The negative
temperature feedback results in a top-peaked power distribution. The fuel temperature
peaks near the mid-plane due to the combined effects of the relatively high power
density and the continually-increasing fluid temperature with distance from the inlet. The pressure gradient is nearly
constant with axial position.
Due to mass conservation, the heating of the fluid results in the velocity
increasing with distance from the inlet.

!media assembly_averages.png
  id=assembly_averages
  caption=Radially-averaged temperatures, power, pressure, and velocity as a function of position.
  style=width:80%;margin-left:auto;margin-right:auto
