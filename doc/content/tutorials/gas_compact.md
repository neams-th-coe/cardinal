# TRISO Compacts

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE for [!ac](TRISO) fuels
- Establish coupling between OpenMC and MOOSE for nested universe OpenMC models
- Apply homogenized temperature feedback to heterogeneous OpenMC cells

To access this tutorial,

```
cd cardinal/tutorials/gas_compact
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
For testing purposes, you may choose to decrease the number of particles to
solve faster.
!alert-end!

Cardinal contains convenient features for applying multiphysics
feedback to heterogeneous domains, when a coupled physics application (such as the MOOSE
heat conduction module) might *not* also resolve the heterogeneities. For instance, the
fuel pebble model in Pronghorn
[!cite](novak2021b) uses the Heat Source Decomposition method to predict pebble
interior temperatures, which does not explicitly resolve the [!ac](TRISO) particles
in the pebble. However, it is usually important to explicitly resolve the particles
for Monte Carlo simulations, to correctly capture self-shielding.
Cardinal allows temperatures to be applied to OpenMC cells
that contain nested universes or lattices by recursing through all the cells
filling the given cell and setting the temperature of all contained cells.
This tutorial describes how to use this feature for
coupling of OpenMC to MOOSE for a [!ac](TRISO)
fuel compact. This example only considers coupling of OpenMC to the solid
phase - in [a later tutorial](triso_multiphysics.md), we extend this example to consider
feedback with the fluid phase as well.

This tutorial was developed with support from the NEAMS Thermal Fluids
Center of Excellence. A journal article [!cite](novak2022_cardinal) describing the physics models,
mesh refinement studies, and auxiliary analyses provides additional context
and application examples beyond the scope of this tutorial.

## Geometry and Computational Model

The geometry consists of a unit cell of a [!ac](TRISO)-fueled
gas reactor compact
[!cite](sterbentz).
A top-down view of the geometry is shown in
[unit_cell]. The fuel is cooled by helium flowing in a cylindrical channel
of diameter $d_c$. Cylindrical fuel compacts containing randomly-dispersed
[!ac](TRISO) particles at 15% packing fraction
are arranged around the coolant channel in a triangular
lattice; the distance between the compact and coolant channel centers
is $p_{cf}$. The diameter of the fuel compact cylinders is $d_f$.
The [!ac](TRISO) particles use a conventional design that consists of a central
fissile uranium oxycarbide kernel enclosed in a carbon buffer, an inner
[!ac](PyC) layer, a silicon carbide layer, and finally an outer
[!ac](PyC) layer. The geometric specifications are summarized in [table1].
Heat is produced in the [!ac](TRISO) particles to yield a total power of 30 kW.

!media compact_unit_cell.png
  id=unit_cell
  caption=[!ac](TRISO)-fueled gas reactor compact unit cell
  style=width:60%;margin-left:auto;margin-right:auto

!table id=table1 caption=Geometric specifications for a [!ac](TRISO)-fueled gas reactor compact
| Parameter | Value (cm) |
| :- | :- |
| Coolant channel diameter | 1.6 |
| Fuel compact diameter | 1.27 |
| Fuel-to-coolant center distance | 1.628 |
| Height | 160 |
| TRISO kernel radius | 214.85e-4 |
| Buffer layer radius | 314.85e-4 |
| Inner PyC layer radius | 354.85e-4 |
| Silicon carbide layer radius | 389.85e-4 |
| Outer PyC layer radius | 429.85e-4 |

### Heat Conduction Model

!include steady_hc.md

The solid mesh is shown in [solid_mesh]; the only sideset defined in the domain
is the coolant channel surface. The
solid geometry uses a length unit of meters.

!media compact_solid_mesh2.png
  id=solid_mesh
  caption=Mesh for the solid heat conduction model
  style=width:60%;margin-left:auto;margin-right:auto

This mesh is generated using MOOSE mesh generators in the `mesh.i` file.

!listing /tutorials/gas_compact/mesh.i
  block=Mesh

We first create a full 7-pin bundle,
and then apply a trimming operation to split the compacts. Because MOOSE does not
support multiple element types (e.g. tets, hexes) on the same block ID, the trimmer
automatically creates an additional block (`compacts_trimmer_tri`) to represent
the triangular prism elements formed in the compacts.
You can generate this mesh by running

```
cardinal-opt -i mesh.i --mesh-only
```

which will create the mesh, named `mesh_in.e`.

Because this tutorial only considers solid coupling, no fluid flow and heat transfer in the
helium is modeled. Therefore, heat removal by the fluid is approximated by setting the
coolant channel surface to a Dirichlet temperature condition of $T_s=T_f$, where $T_f$ is given as

\begin{equation}
\label{eq:fluid}
2\pi\frac{d_c^2}{4}\int_0^z q(l)dl=\dot{m}C_{p,f}\left(T_f(z)-T_{inlet}\right)
\end{equation}

where $q$ is the fission volumetric power density, $\dot{m}$ is the mass flowrate, $C_{p,f}$ is the fluid
isobaric specific heat capacity, $T_f$ is the fluid temperature, and $T_{inlet}$ is the fluid inlet temperature. Although we
will be computing power with OpenMC, just for the sake of applying a fluid temperature boundary
condition, we assume the axial power distribution is sinusoidal,

\begin{equation}
\label{eq:q}
q(z)=q_0\sin{\left(\frac{\pi z}{H}\right)}
\end{equation}

where $H$ is the compact height and $q_0$ is a constant to obtain the total specified power
of 30 kW. The nominal fluid mass flowrate is 0.011 kg/s and the inlet temperature is
325&deg;C. All other boundaries in the solid domain are insulated.
We will run the OpenMC model first, so the only initial condition required for the
solid model is an initial temperature of 325&deg;C.

### OpenMC Model

The OpenMC model is built using [!ac](CSG). The [!ac](TRISO) positions are
sampled using the [!ac](RSA) [algorithm in OpenMC](https://docs.openmc.org/en/stable/examples/triso.html).
OpenMC's Python [!ac](API) is
used to create the model with the script shown below. First, we define materials
for the various regions. Next, we create a single [!ac](TRISO) particle universe
consisting of the five layers of the particle and an infinite extent of graphite
filling all other space. We then pack pack uniform-radius spheres into a cylindrical
region representing a fuel compact, setting each sphere to be filled with the
[!ac](TRISO) universe.

!listing /tutorials/gas_compact/unit_cell.py language=python

Finally, we loop over
$n_l$ axial layers and create unique cells for each of the six compacts, the graphite
block, and the coolant. The level on which we will apply
feedback from MOOSE is set to 1 because each layer is a component in a lattice nested once
with respect to the highest level. To accelerate the particle tracking, we:

- Repeat the same [!ac](TRISO) universe in each axial layer and within each compact
- Superimpose a Cartesian search lattice in the fuel channel regions.

The OpenMC geometry, colored by cell ID,
is shown in [openmc_model].
The lateral faces of the unit cell are periodic, while the top
and bottom boundaries are vacuum. The Cartesian search lattice in the fuel compact
regions is also visible.

!media compact_cells.png
  id=openmc_model
  caption=OpenMC model, colored by cell ID
  style=width:60%;margin-left:auto;margin-right:auto

Because we will run OpenMC first, the initial temperature is set to a uniform
distribution in the $x$-$y$ plane with the axial distribution given by
[eq:fluid]. The fluid density is set using a helium correlation at a fixed pressure
of 7.1 MPa [!cite](petersen) given the imposed temperature, i.e. $\rho_f(P, T)$.

To create the XML files required to run OpenMC, run the script:

```
python unit_cell.py
```

You can also use the XML files checked in to the `tutorials/gas_compact` directory.

## Multiphysics Coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback
for the solid regions of a [!ac](TRISO)-fueled gas reactor compact.
The following sub-sections
describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described in
the `solid.i` input. We define a number of constants at the beginning of the file and
set up the mesh from a file.

!listing /tutorials/gas_compact/solid.i
  end=Variables

Next, we define the temperature variable, `T`, and specify the governing equations and
boundary conditions we will apply.

!listing /tutorials/gas_compact/solid.i
  start=Variables
  end=AuxVariables

The MOOSE heat conduction module will receive power from OpenMC in the form of
an [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html),
so we define a receiver variable for the fission power, as `power`. We also define
a variable `fluid_temp`, that we will use a [FunctionIC](https://mooseframework.inl.gov/source/ics/FunctionIC.html)
to set to the distribution in [eq:fluid].

!listing /tutorials/gas_compact/solid.i
  start=AuxVariables
  end=Functions

We use functions to define the thermal conductivities and the fluid temperature.
The thermal conductivity of the fuel compacts is computed as a volume average of
the materials present.

!listing /tutorials/gas_compact/solid.i
  start=Functions
  end=Postprocessors

We define a number of postprocessors for querying the solution as well as for
normalizing the fission power, to be described at greater length in
[#neutronics_files].

!listing /tutorials/gas_compact/solid.i
  block=Postprocessors

Even though there are no time-dependent kernels in [eq:hc], we use a
[Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
such that each Picard iteration between OpenMC and MOOSE heat conduction
is essentially one "pseduo" time step
("pseudo" because neither the OpenMC or solid model have any notion of time derivatives).

!listing /tutorials/gas_compact/solid.i
  start=Executioner
  end=UserObjects

Finally, for additional postprocessing we use a
[NearestPointLayeredAverage](https://mooseframework.inl.gov/source/userobject/NearestPointLayeredAverage.html)
user object to perform averages in the $x$-$y$ plane in 30 equal-size layers
oriented in the $z$ direction. We separate the average by blocks in order
to compute separate averages for fuel and graphite temperatures.
The layer-wise averages are then exported to the
CSV output (in vector postprocessor form) by using two
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html).
Note that these user objects are strictly for visualization and generating
the plot in [temperature_axial] - temperatures sent to OpenMC will be
taken from the `T` variable.

!listing /tutorials/gas_compact/openmc.i
  start=UserObjects
  end=Executioner

### Neutronics Input Files
  id=neutronics_files

The neutronics physics is solved over the entire domain with OpenMC. The OpenMC
wrapping is described in the `openmc.i` file. We begin by defining a number of constants
and by setting up the mesh mirror on which OpenMC will receive temperature from the
coupled MOOSE application, and on which OpenMC will write the fission heat source.
Because the coupled MOOSE application uses length units of meters, the mesh mirror
must also be in units of meters in order to obtain correct data transfers.
For simplicity, we just use the same mesh used for solution of the solid heat conduction,
though a different mesh could also be used.

!listing /tutorials/gas_compact/openmc.i
  end=AuxVariables

Next, for visualization purposes
we define an auxiliary variable and apply a [CellTemperatureAux](/auxkernels/CellTemperatureAux.md)
in order to get the temperature imposed on the OpenMC cells.
This auxiliary variable will then display the volume-averaged temperature
mapped to the OpenMC cells.

!listing /tutorials/gas_compact/openmc.i
  start=AuxVariables
  end=ICs

The `[Problem]` block is then used to specify settings for the OpenMC wrapping. We define a total power
of 30 kW and indicate that we'd like to add tallies to the fuel compacts. The cell
tally setup in Cardinal will then automatically add a tally for each unique cell ID+instance
combination. By setting `solid_blocks` to all blocks, OpenMC will then receive temperature
from MOOSE for the entire solid domain (because the mesh mirror consists of these
three blocks). Importantly, note that the `[Mesh]` must always be in units that match
the coupled MOOSE application. But because OpenMC solves in units of centimeters,
we specify a `scaling` of 100, i.e. a multiplicative factor to apply to the
`[Mesh]` to get into OpenMC's centimeter units.

!listing /tutorials/gas_compact/openmc.i
  block=Problem

Other features we use include an output of the fission tally standard deviation
in units of W/m$^3$ to the `[Mesh]` by setting the `output` parameter. This is used to
obtain the standard deviation of the heat source distribution from OpenMC in the same
units as the heat source. We also leverage a helper utility in Cardinal by setting
`check_equal_mapped_tally_volumes` to `true`. This parameter will throw an error if
the tallied OpenMC cells map to different volumes in the MOOSE domain. Because we know
*a priori* that the equal-volume OpenMC tally cells *should* all map to equal volumes, this will
help ensure that the volumes used for heat source normalization are also all equal.
For further discussion of this setting and a pictorial description of the
effect of non-equal mapped volumes, please see the
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) documentation.

Because
the OpenMC model is formed by creating axial layers in a lattice nested one level
below the highest universe level, the solid cell level is set to 1. Because the fuel compacts
contain [!ac](TRISO) particles, this indicates that *all* cells in a fuel compact "underneath"
level 1 will be set to the same temperature. Because the fuel compacts are homogenized in the
heat conduction model, this multiphysics coupling is just an approximation to the true
physics. Cardinal supports resolved [!ac](TRISO) multiphysics coupling,
provided the solid mesh explicitly resolves the [!ac](TRISO) particles.

Because OpenMC is coupled by temperature to MOOSE, Cardinal automatically
adds a variable named `temp` that will be an intermediate receiver of temperatures from
MOOSE (before volume averaging by cell within [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)).
We set an initial condition for temperature in OpenMC by setting a
[FunctionIC](https://mooseframework.inl.gov/source/ics/FunctionIC.html) to `temp`
with the function given by [eq:fluid].

!listing /tutorials/gas_compact/openmc.i
  start=ICs
  end=Problem

We run OpenMC as the main application, so we next need to define a
[MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html) that will run
the solid heat conduction model as the sub-application. We also require two transfers. To get the fission
power into the solid model, we use a [MultiAppShapeEvaluationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppShapeEvaluationTransfer.html)
and ensure conservation of the total power by specifying postprocessors
to be preserved in the OpenMC wrapping (`heat_source`) and in the sub-application
(`power`). To get the solid temperature into the OpenMC model, we also use
a [MultiAppShapeEvaluationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppShapeEvaluationTransfer.html)
in the reverse direction.

!listing /tutorials/gas_compact/openmc.i
  start=MultiApps
  end=Postprocessors

We define a number of postprocessors to query the solution. The
[TallyRelativeError](/postprocessors/TallyRelativeError.md)
extracts the maximum fission tally relative error for monitoring active cycle convergence.
The `max_power` and `min_power` [ElementExtremeValue](https://mooseframework.inl.gov/source/postprocessors/ElementExtremeValue.html)
postprocessors compute the maximum and minimum fission power. And as already
discussed, the `heat_source` postprocessor computes the total power
for normalization purposes.

!listing /tutorials/gas_compact/openmc.i
  block=Postprocessors

We use a [NearestPointLayeredAverage](https://mooseframework.inl.gov/source/userobject/NearestPointLayeredAverage.html)
to radially average the OpenMC heat source
and its standard deviation in axial layers across the 6 compacts.
We output the result to CSV using
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html). Note that this user object is strictly used for visualization purposes
to generate the plot in [heat_source] - the heat source applied to the MOOSE
heat conduction model is taken from the `heat_source` variable transferred with
the `heat_source_to_solid` transfer.

!listing /tutorials/gas_compact/openmc.i
  start=UserObjects
  end=Executioner

This input will run OpenMC and the MOOSE heat conduction model in Picard iterations
via pseudo time-stepping. We specify a fixed number of time steps by running
OpenMC with a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner. Finally, we specify Exodus and CSV output formats.

!listing /tutorials/gas_compact/openmc.i
  start=Executioner

## Execution and Postprocessing

To run the coupled calculation,

```
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

This will run both MOOSE and OpenMC with 2 [!ac](MPI) processes and 2 OpenMP threads per rank.
When the simulation has completed, you will have created a number of different output files:

- `openmc_out.e`, an Exodus file with the OpenMC solution and the data that was
  ultimately transferred in/out of OpenMC
- `openmc_out_solid0.e`, an Exodus file with the solid solution
- `openmc_out.csv`, a CSV output with the postprocessors from `openmc.i`
- `openmc_out_solid0.csv`, a CSV output with the postprocessors from `solid.i`
- `openmc_out_avg_q_<n>.csv`, CSV output at time step `<n>` with the
   radially-averaged fission power
- `openmc_out_stdev_<n>.csv`, CSV output at time step `<n>` with the
   radially-averaged fission power standard deviation
- `openmc_out_solid0_block_axial_avg_<n>.csv`, CSV output at time step `<n>` with
  the radially-average block temperature
- `openmc_out_solid0_fuel_axial_avg_<n>.csv`, CSV output at time step `<n>` with
  the radially-average fuel temperature

First, let's examine how the mapping between OpenMC and MOOSE was established.
When we run with `verbose = true`, you will see the following mapping information
displayed:

!include logfile.md

The cells with instances ranging from 0 to 5 represent the six fuel compacts
and the graphite surrounding the fuel compacts (in those hex lattice positions). The other
cells represent the single pure-graphite cell immediately hugging the coolant channel
(the 0, 0 position in the hex lattice). [heat_source] shows the heat source computed by OpenMC
on the mesh mirror (left) and radially averaged as a function of axial position (right).
The error bars on the heat source are smaller than the marker size, so are not shown.
Due to the negative temperature coefficient, the power distribution is shifted
slightly downwards towards regions of lower temperature.

!media compact_heat_source.png
  id=heat_source
  caption=Heat source computed by OpenMC on the mesh mirror (left) and radially averaged as a function of axial position (right)

[temperature] shows the temperature computed by MOOSE and the temperature applied to
the OpenMC cells both as volume plots (left) and as slices at the top of the unit cell
(right). In the volume plots, a zoomed-in view of the temperature near the midplane
is shown on a different color scale to better illustrate the high temperatures in the
compacts and the lower temperatures in the surrounding graphite block.

In the right image, black lines denote the boundaries of the OpenMC cells
mapped to the solid domain. The inner hexagon enclosing the coolant channel is shown
as the sage green color in [openmc_model]. For each unique cell ID+instance combination,
a unique volume-average temperature is performed of the MOOSE solution according to the
mapping of element centroids to the OpenMC cells. Therefore, on any given $x$-$y$ plane,
38 temperatures are applied to the OpenMC model

- one temperature for each of the fuel compacts (6)
- one temperature for the graphite surrounding each fuel compact (6)
- one temperature for the graphite surrounding the coolant channel (1)

The inset in the bottom right image in [temperature] shows the temperatures imposed
in each fuel compact, on a different color scale. Although the
geometry contains several planes of symmetry, because we created unique tallies for
each fuel compact (which have a small uncertainty of less than 1%), there are
small asymmetries less than 0.4 K in magnitude in the temperature in the six compacts.

!media compact_temperature.png
  id=temperature
  caption=Temperature computed by MOOSE and applied to the OpenMC cells as volumes (left) and as slices at the top of the unit cell (right).

[temperature_axial] shows the fuel and graphite block temperatures as a function of
axial position; each point is obtained by averaging over a slice parallel to the
$x$-$y$ plane. The graphite block temperature largely follows the imposed fluid
temperature distribution, while the fuel compact temperature reaches a peak below
the outlet because the fission power distribution is highest just slightly
below the core mid-plane, as shown in [heat_source].

!media compact_axial_T.png
  id=temperature_axial
  caption=Radially-averaged fuel and graphite block temperatures in 30 axial layers
  style=width:50%;margin-left:auto;margin-right:auto

!bibtex bibliography
