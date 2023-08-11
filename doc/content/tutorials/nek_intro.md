# Flow Over a Pebble

In this tutorial, you will learn how to:

- Generate a mesh for NekRS
- Create NekRS input files for laminar flow
- Visualize NekRS outputs

To access this tutorial,

```
cd cardinal/tutorials/pebble_1
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
!alert-end!

## Geometry and Computational Model

The domain consists of a single pebble within a rectangular box; the pebble
origin is at $(0, 0, 0)$. [pebble_1] shows the fluid portion of the domain (the
pebble is not shown). The sideset numbering in the fluid domain is:

- 1: inlet
- 2: outlet
- 3: lateral walls
- 4: pebble surface

!media pebble_1.png
  id=pebble_1
  caption=NekRS flow domain. Sidesets in the fluid domain are colored on the right.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

NekRS shall solve for laminar flow over this pebble; the Reynolds number is $Re=50$
and the Prandtl number is $Pr=0.7$.
We will also apply a uniform heat flux on the pebble surface. In the
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

Here, the NekRS case will be set up in [non-dimensional form](nondimensional_ns.md).
This is a convenient technique for fluid solvers, because the solution will be correct
for *any* flow at a given Reynolds/Peclet/etc. number in this geometry. A common user
operation is also to "ramp" up a CFD solve for stability purposes. Solving in non-dimensional
form means that you do not need to also "ramp" absolute values for velocity/pressure/temperature/etc. - switching to a different set of non-dimensional numbers is as simple
as just modifying the thermophysical "properties" of the fluid (e.g. viscosity, conductivity, etc.). Later, the [7-pin bundle tutorial](tutorials/cht2.md) will solve
NekRS in dimensional form so that you can compare the difference from this tutorial.

### .re2 Mesh

NekRS uses a mesh in a custom `.re2` format.

!alert note
NekRS has some restrictions on what constitutes a valid mesh:
- Mesh must have hexahedral elements in either Hex8 (8-node hexahedral) or Hex20 (20-node hexahedral) forms.
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

Now that you have `exo2nek`, we are ready to convert your mesh (`pebble.exo`) into `.re2` format. Run the following from the `pebble_1`	directory:

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
An illustration for a 5th-order NekRS solution is shown in [gll] for a 2-D element. Each
red dot is a node ([!ac](GLL) quadrature). When you create a mesh for NekRS, you do not
see these [!ac](GLL) points in your starting Exodus/Gmsh mesh. Instead, they will be created
when you launch NekRS (in other words, you do not need to create unique meshes if you want
to run NekRS at different polynomial orders).

!media gll.png
  id=gll_mesh
  caption=Illustration of nodal positions for a $N=5$ polynomial solution.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

### .par File

The `.par` file is used to set up the high-level settings for the case.
This file consists of blocks (in square brackets) and parameters.

!include /tutorials/pebble_1/pebble.par

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
Everything in this file is in non-dimensional form - i.e. the time step size,
the fluid density, etc. A NekRS output file is written every 5000 time steps.
The `stopAt` and `numSteps` fields indicate when to stop the simulation (after 20000 time steps).
We will use a polynomial order of $N=5$.

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
and `viscosity` is used to specify viscosity. When the Navier-Stokes equations
are written in [non-dimensional form](nondimensional_ns.md), the "density" becomes unity because

\begin{equation}
\label{eq:nondim_p}
\rho^\dagger\equiv\frac{\rho_f}{\rho_0}=1
\end{equation}

The "viscosity" becomes the coefficient on the
viscous stress term ($1/Re$, where $Re$ is the Reynolds number).
In NekRS, specifying `diffusivity = -50.0` is equivalent to specifying
`diffusivity = 0.02` (i.e. $1/50.0$), or a Reynolds number of 50.0.

In the `[TEMPERATURE]` block, `rhoCp` indicates the coefficient on
the time derivative term (volumetric specific heat). In non-dimensional form,
this term becomes unity because

\begin{equation}
\label{eq:nek1}
\rho^\dagger C_{p,f}^\dagger\equiv\frac{\rho_fC_{p,f}}{\rho_0C_{p,0}}=1
\end{equation}

The `conductivity` indicates the coefficient on the diffusion kernel, which in non-dimensional
form is equal to $1/Pe$, where $Pe$ is the Peclet number. In NekRS, specifying `conductivity = -35` is equivalent
to specifying `conductivity = 0.02857` (i.e. $1/35.0$), or a Peclet number of 35. Note that
$Pe\equiv Pr \cdot Re$, so our case is solving for fluid flow at $Pr=0.7$.

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
- `bc->s` is the scalar (temperature) solution at the present [!ac](GLL) point
- `bc->flux` is the flux (of temperature) at the present [!ac](GLL) point

!listing /tutorials/pebble_1/pebble.oudf language=cpp

Here, we set a uniform inlet velocity of $V_z=1$ (non-dimensional), a zero value for temperature
(non-dimensional), and a heat flux of 1.0 (non-dimensional).

### .udf File

The `.udf` file contains additional C++ code which is typically used for setting initial conditions,
postprocessing, and adding custom physics kernels (not used in this tutorial). This file at a minimum
must contain 3 functions (they can be empty):

- `UDF_LoadKernels` will load any user-defined physics kernels
- `UDF_Setup` is called on initialization time, and is typically where initial conditions are applied
- `UDF_ExecuteStep` is called once on each time step, and is typically where postprocessing is applied

!listing /tutorials/pebble_1/pebble.udf language=cpp

Here, we only use the `UDF_Setup` function to set initial conditions (to the same values
we applied as our inlet boundary conditions). NekRS stores velocity as `nrs->U` (all three
components are packed one after another, with each "slice" of length `nrs->fieldOffset`).
Pressure is represented as `nrs->P`, and the passive scalars (of which our problem only has one,
i.e. temperature) are stored in `nrs->cds->S` (packed sequentially one after another, each
of length `nrs->cds->fieldOffset[i]`). We apply the initial condition by looping over all the
elements in the mesh, and in each element for all the [!ac](GLL) points.

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

These files (often referred to as "field" files) need to be accompanied by a small
"configuration" file in order to load into Paraview. To generate that file, we need to
use the `visnek` script like so

```
cardinal/contrib/nekRS/3rd_party/nek5000/bin/visnek pebble
```

(We recommend adding that location to your path). This will create a file named
`pebble.nek5000`, which has very simple contents:

```
 filetemplate: pebble%01d.f%05d
 firsttimestep: 1
 numtimesteps:            20
```

To open the NekRS output files, you then need to open the `pebble.nek5000` file.

!alert note
To open the files in Paraview or Visit, you must also be sure to
have co-located with the `pebble.nek5000` files your actual output files from NekRS
(e.g. the `pebble0.f<n>` files).


!bibtex bibliography
