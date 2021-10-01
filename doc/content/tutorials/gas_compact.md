# Tutorial 6C: TRISO Compacts

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE
- Establish coupling between OpenMC and MOOSE for nested universe OpenMC models
- Apply homogenized temperature feedback to heterogeneous OpenMC cells

!alert! note
This tutorial makes use of the following major Cardinal classes:

- [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)

We recommend quickly reading this documentation before proceeding
with this tutorial. This tutorial also requires you to download a
mesh file from Box. Please download the files from the
`gas_compact` folder [here](https://anl.app.box.com/folder/141527707499?s=irryqrx97n5vi4jmct1e3roqgmhzic89)
and place these files within the same directory structured
in `tutorials/gas_compact`.
!alert-end!

Cardinal contains convenient features for applying multiphysics
feedback to heterogeneous domains, when a coupled physics application (such as MOOSE
heat conduction) might *not* also resolve the heterogeneities. For instance, the
fuel pebble model in the coarse-mesh thermal-hydraulic tool Pronghorn
[!cite](novak2021b) uses the Heat Source Decomposition method to predict pebble
interior temperatures, which does not explicitly resolve the [!ac](TRISO) particles
in the pebble. Cardinal allows temperatures to be applied to OpenMC cells
that contain nested universes or lattices by recursing through all the cells
filling the given cell and setting the temperature of all contained cells.
This tutorial describes how to use this feature for homogenized
temperature and heat source coupling of OpenMC to MOOSE for a [!ac](TRISO)
fueled gas compact. This example only considers coupling of OpenMC to the solid
phase - in [Tutorial 9B](triso_multiphysics.md), we extend this example to consider
feedback with the fluid phase as well.

## Geometry and Computational Model

The geometry for this example consists of a unit cell of a [!ac](TRISO)-fueled
gas reactor compact, loosely based on a point design available in the literature
[!cite](sterbentz).
A top-down view of the geometry is shown in
[unit_cell]. The fuel is cooled by helium flowing in a cylindrical channel
of diameter $d_c$. Cylindrical fuel compacts containing randomly-dispersed
[!ac](TRISO) particles at 15% packing fraction
are arranged around the coolant channel in a triangular
lattice; the distance between the compact and coolant channel centers
is $p_{cf}$. The diameter of the fuel compact cylinders is $d_f$.
The [!ac](TRISO) particles use a conventional design that consists of a central
fissil uranium oxycarbide kernel enclosed in a carbon buffer, an inner
[!ac](PyC) layer, a silicon carbide layer, and finally an outer
[!ac](PyC) layer. The geometric specifications are summarized in [table1].
Heat is produced in the [!ac](TRISO) particles; the total power is 30 kW.

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

## Heat Conduction Model

The MOOSE heat conduction module is used to solve for steady-state heat conduction,

\begin{equation}
\label{eq:hc}
-\nabla\cdot\left(k\nabla T\right)=\dot{q}
\end{equation}

where $k$ is the thermal conductivity, $T$ is the temperature, and $\dot{q}$ is a volumetric
heat source. The solid mesh is shown in [solid_mesh]; the only sideset defined in the domain
is the coolant channel surface.
To simplify the specification of
material properties, the solid geometry uses a length unit of meters.

!media compact_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid heat conduction model
  style=width:60%;margin-left:auto;margin-right:auto

Because this tutorial only considers solid coupling, no fluid flow and heat transfer in the
helium is modeled. Therefore, heat removal by the fluid is approximated by setting the
coolant channel surface to a Dirichlet temperature condition,

\begin{equation}
\label{eq:fluid}
2\pi\frac{d_c^2}{4}\int_0^z q(l)dl=\dot{m}C_{p,f}\left(T(z)-T_{inlet}\right)
\end{equation}

where $q$ is the fission volumetric power density, $\dot{m}$ is the mass flowrate, $C_{p,f}$ is the fluid
isobaric specific heat capacity, and $T_{inlet}$ is the fluid inlet temperature. Although we
will be computing power with OpenMC, just for the sake of applying a fluid temperature boundary
condition, we assume the axial power distribution is sinusoidal,

\begin{equation}
\label{eq:q}
q(z)=q_0\sin{\left(\frac{\pi z}{H}\right)}
\end{equation}

where $H$ is the compact height and $q_0$ is a constant to obtain the total specified power
of 30 kW. The nominal fluid mass flowrate is 0.011 kg/s and the inlet temperature is
325&deg;C. All other boundaries in the solid domain are insulated.
We will run the OpenMC model first, and because the solid model uses a steady equation,
the initial condition for the solid temperature is inconsequential.

## OpenMC Model

The OpenMC model is built using [!ac](CSG) geometry. The [!ac](TRISO) positions are
sampled using the [!ac](RSA) algorithm in OpenMC. OpenMC's Python [!ac](API) is
used to create the model with the script shown below. First, we define materials
for the various regions. Next, we create a single [!ac](TRISO) particle universe
consisting of the five layers of the particle and an infinite extent of graphite
filling all other space. We then pack pack uniform-radius spheres into a cylindrical
region representing a fuel compact, setting each sphere to be filled with the
[!ac](TRISO) universe.

!listing /tutorials/gas_compact/unit_cell.py language=python

Finally, we loop over
$n_l$ axial layers and create unique cells for each of the six compacts, the graphite
block, and the coolant. This means that each fuel compact and graphite block receives
a unique temperature from MOOSE in each axial layer. The level on which we will apply
feedback from MOOSE is 1, because each layer is a component in a lattice nested once
with respect to the highest level. To accelerate the particle tracking, we:

- Repeat the same [!ac](TRISO) universe in each axial layer and within each compact
- Superimpose a Cartesian search lattice in the fuel channel regions.

The OpenMC geometry, colored by cell ID,
is shown in [openmc_model].
The lateral faces of the unit cell are periodic, while the top
and bottom boundaries are vacuum.

!media compact_cells.png
  id=openmc_model
  caption=OpenMC model, colored by cell ID
  style=width:60%;margin-left:auto;margin-right:auto

Because we will run OpenMC first, the initial temperature will be set to a uniform
distribution in the $x$-$y$ plane with the axial distribution given by
[eq:fluid]. The fluid density is set using a helium correlation at a fixed pressure
of 7.1 MPa [!cite](petersen) given the imposed temperature, i.e. $\rho_f(P, T)$.

To create the XML files required to run OpenMC, run the script:

```
$ python unit_cell.py
```

You can also use the XML files checked in to the `tutorials/gas_compact` directory

## Multiphysics Coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback
for the solid regions of a [!ac](TRISO)-fueled gas reactor compact. All input files
are present in the `tutorials/gas_compact` directory. The following sub-sections
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
  end=Functions

The MOOSE heat conduction module will receive power from OpenMC in the form of
an [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html),
so we define a receiver variable for the fission power, as `power`. We also define
a variable `fluid_temp`, that we will use a [FunctionIC](https://mooseframework.inl.gov/source/ics/FunctionIC.html)
to set to the distribution in [eq:fluid].

!listing /tutorials/gas_compact/solid.i
  start=AuxVariables
  end=Variables

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

Even though there are no time-dependent kernels in [eq:hc], we must use a
[Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner because the solid physics is run as a sub-application to OpenMC,
which we run multiple times by specifying a number of "pseudo" time steps
("pseudo" because neither the OpenMC or solid model have any notion of time as a
state variable).

Finally, for additional postprocessing we use a
[NearestPointLayeredAverage](https://mooseframework.inl.gov/source/userobject/NearestPointLayeredAverage.html)
user object to perform averages in the $x$-$y$ plane in 30 equal-size layers
oriented in the $z$ direction. The layer-wise averages are then exported to the
CSV output (in vector postprocessor form) by using two
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html).

### Neutronics Input Files
  id=neutronics_files

The neutronics physics is solved over the entire domain with OpenMC. The OpenMC
wrapping is described in the `openmc.i` file. We begin by defining a number of constants
and by setting up the mesh mirror on which OpenMC will receive temperature from the
coupled MOOSE application, and on which OpenMC will write the fission heat source.
Because the coupled MOOSE application uses length units of meters, the mesh mirror
must also be in units of meters in order to obtain correct data transfers.

!listing /tutorials/gas_compact/openmc.i
  end=AuxVariables

Next, for visualization purposes
we define an auxiliary variable and apply a [CellTemperatureAux](/auxkernels/CellTemperatureAux.md)
in order to get the temperature imposed on the OpenMC cells.

!listing /tutorials/gas_compact/openmc.i
  start=AuxVariables
  end=ICs

The `[Problem]` block is then used to specify the OpenMC wrapping. We define a total power
of 30 kW and indicate that we'd like to add tallies to block 2 (the fuel compacts). The cell
tally setup in Cardinal will then automatically add a tally for each unique cell ID+instance
combination. By setting `solid_blocks` to blocks 1 and 2, OpenMC will then receive temperature
from MOOSE for the entire solid domain. Because the `[Mesh]` is in units of meters, but
OpenMC always uses units of centimeters, we specify a `scaling` of 100, or a multiplicative
factor of 100 to apply to the `[Mesh]` to get into OpenMC's centimeter units.

Other features we use include an output of the fission tally standard deviation
in units of W/m$^3$ to the `[Mesh]` by setting the `output` parameter. This is used to
obtain uncertainty estimates of the heat source distribution from OpenMC in the same
units as the heat source. We also leverage a helper utility in Cardinal by setting
`check_equal_mapped_tally_volumes` to `true`. This parameter will throw an error if
the tallied OpenMC cells map to different volumes in the MOOSE domain. Because we know
*a prior* that the OpenMC tally cells *should* all map to equal volumes, this will
help ensure that the volumes used for heat source normalization are also all equal.
For further discussion of this setting and a pictorial description of the
effect of non-equal mapped volumes, please see the
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) documentation.

Because
the [!ac](CSG) geometry is formed by creating axial layers in a lattice nested one level
below the highest universe level, the solid cell level is set to 1. Because the fuel compacts
contain [!ac](TRISO) particles, this indicates that *all* cells in a fuel compact "underneath"
level 1 will be set to the same temperature. Because the fuel compacts are homogenized in the
heat conduction model, this multiphysics coupling is just an approximation to the true
physics, where each layer in the [!ac](TRISO) particles, as well as the graphite matrix, have
different average temperatures. If the solid heat conduction model explicitly resolves
the [!ac](TRISO) particles, then more detailed temperature feedback can be pursued.

Because this wrapping is coupled by temperature to MOOSE, Cardinal automatically
adds a variable named `temp` that will be an intermediate receiver of temperatures from
MOOSE (before volume averaging by cell within [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)).
We set an initial condition for temperature in OpenMC by setting a
[FunctionIC](https://mooseframework.inl.gov/source/ics/FunctionIC.html) to `temp`.

!listing /tutorials/gas_compact/openmc.i
  start=ICs
  end=Problem

We run OpenMC as the master application, we next need to define a
[MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html) that will run
the solid heat conduction model. We also require two transfers. To get the fission
power into the solid model, we use a [MultiAppMeshFunctionTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppMeshFunctionTransfer.html)
and ensure conservation of the total power by specifying postprocessors
to be preserved in the OpenMC wrapping (`heat_source`) and in the sub-application
(`power`). To get the solid temperature into the OpenMC model, we also use
a [MultiAppMeshFunctionTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppMeshFunctionTransfer.html).

!listing /tutorials/gas_compact/openmc.i
  start=MultiApps
  end=Executioner

Finally, we define a number of postprocessors to query the solution. The
[FissionTallyRelativeError](/postprocessors/FissionTallyRelativeError.md)
extracts the maximum fission tally relative error for monitoring active cycle convergence.
The `max_power` and `min_power` [ElementExtremeValue](https://mooseframework.inl.gov/source/postprocessors/ElementExtremeValue.html)
postprocessors compute the maximum and minimum fission power.

!listing /tutorials/gas_compact/openmc.i
  block=Postprocessors

We also define a [NearestPointLayeredAverage](https://mooseframework.inl.gov/source/userobject/NearestPointLayeredAverage.html)
to radially average the OpenMC heat source
and its standard deviation in 30 axial layers across the 6 compacts.
We output the result to CSV using
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html).

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

To run the coupled calculation, run the following from the command line.

```
$ mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=24
```

This will run both MOOSE and OpenMC with 2 [!ac](MPI) processes and 24 OpenMP threads.
When the simulation has completed, you will have created a number of different output files:

- `openmc_out.e`, an Exodus file with the OpenMC solution and the data that was
  ultimately transferred in/out of OpenMC
- `openmc_out_solid0.e`, an Exodus file with the solid solution
- `openmc_out_avg_q_<n>.csv`, CSV output at time step `<n>` with the
   radially-average fission power in 30 axial layers
- `openmc_out_stdev_<n>.csv`, CSV output at time step `<n>` with the
   radially-averaged fission power standard deviation in 30 axial layers
- `openmc_out_solid0_block_axial_avg_<n>.csv`, CSV output at time step `<n>` with
  the radially-average block temperature in 30 axial layers
- `openmc_out_solid0_fuel_axial_avg_<n>.csv`, CSV output at time step `<n>` with
  the radially-average fuel temperature in 30 axial layers

First, let's examine how the mapping between OpenMC and MOOSE was established.
When we run with `verbose = true`, you will see the following mapping information
displayed:

!include logfile.md

The cells with instances ranging from 0 to 5 represent the six fuel compacts. The other
cells represent the graphite block in each layer.  shows the heat source computed by OpenMC
on the mesh mirror (left) and radially averaged as a function of axial position (right).
The radially-averaged plot is created using a separate Python script to postprocess
the CSV output from the [SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html).
Error bars on the heat source are smaller than the marker size.



!bibtex bibliography
