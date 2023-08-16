# Flow Over a Pebble

In this tutorial, you will learn how to:

- Generate a mesh for NekRS
- Create NekRS input files for laminar flow in dimensional form
- Visualize NekRS outputs

To access this tutorial,

```
cd cardinal/tutorials/pebble_1
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
!alert-end!

## Geometry and Computational Model

The domain consists of a single pebble of diameter $d=3$ cm within a rectangular box; the pebble
origin is at $(0, 0, 0)$. [pebble_1] shows the fluid portion of the domain (the
pebble is not shown). The sideset numbering in the fluid domain is:

- 1: inlet
- 2: outlet
- 3: pebble surface
- 4: side walls

!media pebble_1.png
  id=pebble_1
  caption=NekRS flow domain. Sidesets in the fluid domain are colored on the right.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

NekRS shall solve for laminar flow over this pebble. Details on the problem
specifications are given in [table1]. The inlet velocity is specified such that
the Reynolds number is $Re=50$. The pebble surface heat flux is selected to
give a pebble power of approximately 223 W (giving a bulk fluid temperature rise
of 50 K).

!table id=table1 caption=Geometric and operating conditions for the single-pebble flow
| Parameter | Value |
| :- | :- |
| Pebble diameter | 0.03 m |
| Domain height | 0.4306 m |
| Inlet flow area | 0.00064009 m |
| Inlet velocity | 0.001666 m/s |
| Fluid viscosity | 1e-3 Pa-s |
| Fluid density | 1000 kg/m$^3$ |
| Fluid thermal conductivity | 0.6 W/m/K |
| Fluid volumetric specific heat | 4186 J/kg/K |
| Pebble surface heat flux | 78939 W/m$^2$ |

We will apply a uniform heat flux on the pebble surface. In the
[conjugate heat transfer tutorials](tutorials/cht.md), this heat flux will later be
replaced by coupling to MOOSE.

At a minimum, the NekRS model will constitute the following files:

- `pebble.re2`: Mesh file
- `pebble.par`: High-level settings for the solver, boundary
  conditions, and the equations to solve
- `pebble.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `pebble.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and
  source terms

The default case file naming scheme used by NekRS is to use a common "casename"
(in this case, we chose `pebble`), appended by the different file extensions which represents
different parts of the input file setup. You can use a different naming scheme by
setting file names for the mesh, `.udf`, and `.oudf` file in the `.par` file (discussed later).

!alert note
You may also optionally have a Fortran `.usr` file which provides some backend
capability to Nek5000 which has not yet been ported to NekRS. For instructional
purposes, we will show you one use case for the `.usr` file - modifying sideset IDs.

### .re2 Mesh

NekRS uses a mesh in a custom `.re2` format.

!alert note
NekRS has some restrictions on what constitutes a valid mesh:
- Mesh must have hexahedral elements in either Hex8 (8-node hexahedral) or Hex20 (20-node hexahedral) forms. Hex20 is a second-order mesh format. The advantage of using
a second-order format is evident for geometries with curves, like spheres or cylinders. As you refine to higher polynomial order, the mid-side quadrature points are moved to the curve surface to better capture the geometry.
- The sidesets must be numbered sequentially beginning from 1 (e.g. sideset 1, 2, 3, 4, ...).

If you have a mesh in Exodus or Gmsh format,
you can convert that mesh into `.re2` format using [tools that ship with Nek5000](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html). To get these features, you will need to clone Nek5000 somewhere on your system
and then build the `exo2nek` tool (for converting Exodus meshes) or the
`gmsh2nek` tool (for converting Gmsh meshes).

```
git clone https://github.com/Nek5000/Nek5000.git
cd Nek5000/tools
./maketools exo2nek
```

Running the above will place a binary named `exo2nek` at `Nek5000/bin/exo2nek`. We recommend
adding this to your path.

Now that you have `exo2nek`, we are ready to convert your mesh (`pebble.exo`) into `.re2` format. This mesh is Hex8. Run the following:

```
exo2nek
```

Follow the on-screen prompts. For this case, we have:

- 1 fluid exo file,
- which is named `pebble.exo` (you only need to provide `pebble` as the name)
- 0 solid exo files (this is used when NekRS is solving for both fluid and solid domains)
- 0 periodic surface pairs (this is used to apply periodic boundary conditions)
- output file name is `pebble.re2` (you only need to provide `pebble` as the output name)

This will create a mesh named `pebble.re2`.

NekRS is a spectral element code, which means that the solution in each element is represented
as an $N$-th order Lagrange polynomial (in each direction).
An illustration for a 5th-order NekRS solution is shown in [gll_mesh] for a 2-D element. Each
red dot is a node ([!ac](GLL) quadrature). When you create a mesh for NekRS, you do not
see these [!ac](GLL) points in your starting Exodus/Gmsh mesh. Instead, they will be created
when you launch NekRS (in other words, you do not need to create unique meshes if you want
to run NekRS at different polynomial orders).

!media gll.png
  id=gll_mesh
  caption=Illustration of nodal positions for a 5-th order polynomial solution.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

### .par File

The `.par` file is used to set up the high-level settings for the case.
This file consists of blocks (in square brackets) and parameters.

!listing /tutorials/pebble_1/pebble.par

!alert! tip
You can get a comprehensive list of all the syntax supported by running

```
$NEKRS_HOME/bin/nekrs --help par
```
!alert-end!

#### OCCA block

This block is used to specify whether NekRS is running on CPU or GPU (and which
vendor). For our case, we will run this on CPU so we set the backend as CPU.

#### GENERAL block

The `[GENERAL]` block describes the
time stepping, simulation end control, and polynomial order.
A NekRS output file is written every 1000 time steps.
The `numSteps` field indicate how many time steps to take.
We will use a polynomial order of $N=5$. We will begin with our first
time step as $\Delta t=1e-4$ seconds, but allow NekRS to adaptively increase
this to get a target [!ac](CFL) number of 2.0.

#### VELOCITY, PRESSURE, TEMPERATURE blocks

Next, the `[VELOCITY]` and `[PRESSURE]` blocks describe the solution of the
pressure Poisson equation and velocity Helmholtz equations.
The `[TEMPERATURE]` block describes the solution of the
temperature passive scalar equation. In these blocks, `residualTol` is used to indicate
the solver tolerance. In these blocks, you also specify the type of boundary conditions
to apply to each sideset (you only specify boundary conditions in the `[VELOCITY]` and
`[TEMPERATURE]` blocks, because the pressure and velocity solves are really indicating
together a solution to the momentum conservation equation).
The `boundaryTypeMap` is used to specify the mapping of
boundary IDs to types of boundary conditions. NekRS uses short character strings
to represent the type of boundary condition. For velocity, some of the common boundary condition strings are
shown in [vel_bcs]. We will talk about the `.oudf` file shortly.

!table id=vel_bcs caption=Common velocity boundary conditions in NekRS
| Meaning | How to set in `.par` File | Function name in `.oudf` File |
| :- | :- | :- |
| `v` | Dirichlet velocity | `velocityDirichletConditions` |
| `w` | No-slip wall | --- |
| `o` | Outflow velocity + Dirichlet pressure | `pressureDirichletConditions` (nothing needed for velocity outflow) |
| `symx` | symmetry in the $x$-direction | --- |
| `symy` | symmetry in the $y$-direction | --- |
| `symz` | symmetry in the $z$-direction | --- |
| `sym` | general symmetry boundary | --- |

For temperature, some common boundary condition strings are shown in [temp_bcs].

!table id=temp_bcs caption=Common temperature/scalar boundary conditions in NekRS
| Meaning | How to set in `.par` File | Function name in `.oudf` File |
| :- | :- | :- |
| `t` | Dirichlet temperature | `scalarDirichletConditions` |
| `f` | Neumann flux | `scalarNeumannConditions` |
| `I` | insulated | --- |
| `symx` | symmetry in the $x$-direction | --- |
| `symy` | symmetry in the $y$-direction | --- |
| `symz` | symmetry in the $z$-direction | --- |
| `sym` | general symmetry boundary | --- |

When you populate `boundaryTypeMap` in the input file, you simply list the
character string for your desired boundary condition in the same order as the sidesets
(which as you recall are numbered sequentially).

In the `[VELOCITY]` block, the `density` parameter is used to specify density,
and `viscosity` is used to specify viscosity.
In the `[TEMPERATURE]` block, `rhoCp` indicates the coefficient on
the time derivative term (volumetric specific heat) and `conductivity` indicates
the thermal conductivity.

### .oudf File

The `.oudf` file contains all of the boundary conditions. For more complex
models, this file will in general contains any code which runs on the GPU
(custom physics, etc.).

When you defined the boundary
condition types in the `.par` file, you also need to set the *values* of those boundary conditions (the non-trivial ones)
n the `.oudf` file. The names of these functions are pre-defined by NekRS,
and are paired up to the character strings you set earlier. These function names are shown in
[vel_bcs] and [temp_bcs] in the `.oudf` column. Note that a `---` indicates that no user-defined kernels are necessary
to define that boundary condition (i.e. NekRS already has all the info it needs
to apply a no-slip boundary condition - it just sets velocity to zero).

For each of these functions, the `bcData` object contains all information
about the current boundary that is "calling" the function:

- `bc->id` is the boundary ID
- `bc->u` is the $x$-velocity
- `bc->v` is the $y$-velocity
- `bc->w` is the $z$-velocity
- `bc->s` is the scalar (temperature) solution at the present quadrature point
- `bc->flux` is the flux (of temperature) at the present quadrature point
- `bc->idM` is the index corresponding to the current quadrature point

!listing /tutorials/pebble_1/pebble.oudf language=cpp

### .udf File

The `.udf` file contains additional C++ code which is typically used for setting initial conditions and
postprocessing. This file at a minimum
must contain 3 functions (they can be empty):

- `UDF_LoadKernels` will load any user-defined physics kernels
- `UDF_Setup` is called on initialization time, and is typically where initial conditions are applied
- `UDF_ExecuteStep` is called once on each time step, and is typically where postprocessing is applied

!listing /tutorials/pebble_1/pebble.udf language=cpp

Here, we only use the `UDF_Setup` function to set initial conditions (to the same values
we applied as our inlet boundary conditions). NekRS stores the various solution components on the host as

- `nrs->U` is velocity (all three components are packed one after another, with each "slice" of length `nrs->fieldOffset`)
- `nrs->P` is pressure
- `nrs->cds->S` are the passive scalars (packed sequentially one after another, each
  of length `nrs->cds->fieldOffset[i]`)

Our problem only has one passive scalar (temperature). You will have additional
passive scalars if doing [!ac](RANS) modeling (e.g. $k$ and $\tau$ would be passive
scalars) or if you have additional conservation equations for mass transport, etc.
We apply the initial condition by looping over all the
elements in the mesh, and in each element for all the [!ac](GLL) points.

### .usr File

NekRS is under rapid development, and many features from Nek5000 are currently
being ported over to NekRS. To access those Fortran-based features, you can optionally
include a `.usr` file, which contains funtions for setting initial conditions,
modifying the mesh, etc. Here, we will show you how to use this file for changing
the sideset IDs in our mesh. Our input mesh, `pebble.exo`, did not order the sidesets
beginning from 1; instead, the sidesets were numbered as 2, 3, 4, and 5. So, we would
like to modify those sideset numbers so that they are 1, 2, 3, 4.

We perform this modification in the `usrdat2()` function (the other functions
must be present, but can be empty).

!listing /tutorials/pebble_1/pebble.usr language=fortran

The various terms in this function are:

- `iel` is a loop variable name used to indicate a loop over elements
- `ifc` is a loop variable name used to indicate a loop over the faces on an element
- `boundaryID(ifc, iel)` is the sideset ID for velocity

## Execution and Postprocessing

When you compile Cardinal, you automatically also get a standalone NekRS executable.
You can run the NekRS case with

```
$NEKRS_HOME/bin/nrsmpi pebble 4
```

where `pebble` is the case name and `4` is the number of MPI ranks you would like to use.
Note that NekRS does not use any shared memory parallelism (e.g. OpenMP).
Running this case will produce NekRS output files named
`pebble0.f<n>`, where `<n>` is a 5-digit integer indicating the output file number
(i.e. they will be sequentially-ordered based on when you told NekRS to write output files).

These files (referred to as "field" files) need to be accompanied by a small
"configuration" file in order to load into Paraview. To generate that file, we need to
use the `visnek` script like so

```
cardinal/scripts/visnek pebble
```

This will create a file named `pebble.nek5000`, which has very simple contents:

```
 filetemplate: pebble%01d.f%05d
 firsttimestep: 1
 numtimesteps: 8
```

This file tells Paraview how many time steps there are to load and the naming pattern for those files. To open the NekRS output files, you then need to open the `pebble.nek5000` file.

!alert note
To open the files in Paraview or Visit, you must also be sure to
have co-located with the `pebble.nek5000` files your actual output files from NekRS
(e.g. the `pebble0.f<n>` files).

First, let's take a look at the mesh to get a better sense for how NekRS stores data
in its output files. [zoom_pebble_mesh] shows a zoomed-in view near the pebble surface.
As you can see, this mesh displays the [!ac](GLL) quadrature points. Outlined in red
is a single element in our starting Exodus mesh (`pebble.exo`); the lines inside each
element are connecting the nodes.

!media zoom_pebble_mesh.png
  id=zoom_pebble_mesh
  caption=NekRS spectral mesh, for polynomial order 5
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

Note that the pebble surface is "faceted" in the same way as our starting Exodus mesh.
This is the case because we used a first-order Hex8 mesh.

[standalone_pebble] shows the velocity, pressure, and temperature predicted
by NekRS.

!media standalone_pebble.png
  id=standalone_pebble
  caption=NekRS predicted velocity, pressure, and temperature
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

## Hex8 vs Hex20

To appreciate the difference between Hex8 and Hex20 meshes, [hex20] shows the [!ac](GLL)
points which would be used by NekRS if our input mesh was instead in Hex20 format. As you
can see, the mid-side nodes follow the curvature of the sphere because the (single)
mid-side node in our starting Hex20 mesh "knows about" the curve in the mesh at this point.

!media hex20.png
  id=hex20
  caption=NekRS mesh for polynomial order 5 when using a Hex20 input mesh
  style=width:60%;margin-left:auto;margin-right:auto;halign:center
