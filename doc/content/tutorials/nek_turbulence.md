# Turbulent Flow in a Pipe

In this tutorial, you will learn how to:

- Understand important considerations for running CFD cases
- Generate a periodic flow and heat transfer case with NekRS

To access this tutorial,

```
cd cardinal/tutorials/turbulence
```

## Geometry and Computational Model

The domain consists of a pipe flow. The simulation will be set up in non-dimensional units. The diameter of the pipe is 1 and the length is 10.
Several different Reynolds numbers will be simulated by varying the
fluid viscosity. The sideset numbering in the fluid domain is:

- 1: side walls
- 2: inlet
- 3: outlet

!media pipe.png
  id=pipe
  caption=NekRS flow domain. The inlet is located at $z=0$ and the outlet is located at $z=10$.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

A computational mesh is built using MOOSE's mesh generators, shown
below. To generate this mesh, run

```
cardinal-opt -i pipe.i --mesh-only
mv pipe_in.e pipe.exo
```

!listing /tutorials/nek_turbulence/pipe.i

## Inlet-Outlet Conditions

This tutorial will progress through a series of stages of increasing complexity, first beginning with an inlet-outlet boundary condition. The case files are included in the `inlet_outlet` directory.
We initially run this case for a Reynolds number of 100. The initial
conditions on velocity will be uniform axial velocity of 1 and for temperature
will be a uniform temperature of 0.

!listing /inlet_outlet/pipe.par

!listing /inlet_outlet/pipe.udf

!listing /inlet_outlet/pipe.oudf

Along the pipe wall, we impose a constant heat flux in non-dimensional units of 1.0. When we non-dimensionalize the energy equation, we
divide each term through by the coefficient on the advective term ($\rho C_p\Delta TU/L$), which results in a non-dimensional *volumetric* heat source of

\begin{equation}
\dot{q}^\dagger=\frac{\dot{q}}{\rho C_p\Delta TU/L}
\end{equation}

$\Delta T$ is the temperature difference chosen to scale the dimensional temperature, i.e.

\begin{equation}
\label{eq:scale}
T^\dagger=\frac{T-T_0}{\Delta T}
\end{equation}

Therefore, the reference heat flux to obtain the non-dimensional heat flux is simply $\rho C_p\Delta TU$ (taking the scaling for the volumetric heat and multiplying by a length scale to obtain the correct units). This means that the non-dimensional heat flux is scaled as

\begin{equation}
\frac{q}''^{\dagger}=\frac{q''}{\rho C_p\Delta TU}
\end{equation}

If we apply a non-dimensional heat flux of 1.0, this means that the dimensional heat flux is

\begin{equation}
\label{eq:one}
q''=\rho C_p\Delta TU
\end{equation}

The total energy deposited in the fluid, via this boundary heat flux, is simply the area integral of the imposed heat flux. From bulk conservation of energy, we know that this
energy entering the pipe is equal to $\dot{m}C_p(T_\text{out}-T_\text{in})$.

\begin{equation}
\label{eq:two}
q''2\pi RH=\rho \piR^2 UC_p(T_\text{out}-T_\text{in})
\end{equation}

where $2\pi RH$ is the surface area through which the heat enters and $\pi R^2$ is
the cross-sectional area of the pipe. Inserting Eq. \eqref{eq:one} into Eq. \eqref{eq:two}
and then rearranging, we find the temperature rise (in dimensional units).

\begin{equation}
\Delta T\frac{2H}{R}=T_\text{out}-T_\text{in}
\end{equation}

For scaling temperature, we have not yet specified what $T_0$ and $\Delta T$ are in Eq. \eqref{eq:scale}. Ultimately, these scales do not matter, you can choose any scale! But,
it is customary to choose $T_0=T_\text{in}$ and $\Delta T=T_\text{out}-T_\text{in}$. With
those choices, we can rewrite the right-hand side of the equation above to be in
terms of non-dimensional quantities,

\begin{equation}
\begin{aligned}
\Delta T\frac{2H}{R}=&\ \Delta T T_\text{out}^\dagger+T_\text{in}-(\Delta T T_\text{in}^\dagger+T_\text{in})\\
\frac{2H}{R}=&\ T_\text{out}^\dagger-T_\text{in}^\dagger\\
\end{aligned}
\end{equation}

Therefore, we expect that in non-dimensional units, our outlet bulk temperature will be $2H/R=40$ if our non-dimensional inlet temperature is 0.

Now that we understand what the various non-dimensional quantities represent for
energy, we are ready to run this case.
To generate the `.re2` mesh, we need to run `exo2nek`, which will request
information via console input. For this example, there are no
periodic boundaries or solid portions of the mesh.

```
exo2nek
```

To run the case with 12 MPI ranks (just as an example),

```
nrsmpi fluid 12
```

Because we output one file every 200 steps, we will obtain 10 output files. To view these files, open the `fluid.nek5000` file in a visualization
tool such as Paraview or Visit.

!media pipe_steady.png
  id=pipe_steady
  caption=Time evolution of the fluid temperature
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

### Monitoring Steady State

Because all our boundary conditions are steady, our flow will reach a steady-state
(for laminar flows) or a steady-in-the-mean state (for turbulent flows). We would
refer to either of these states as ``stationarity.'' Generally,
you should expect to need several *convective units* of time to have passed to
reach this condition (though this will depend on the details of the flow and
on what field you are monitoring). A convective unit $t_c$ is the amount of time required
for the flow to move from the inlet to the outlet of the domain one time.

\begin{equation}
t_c=\frac{\text{streamwise length of domain}}{U}
\end{equation}

For our domain, we have a length of 10 and a velocity of 1 (both in non-dimensional units),
so that a convective unit is 10 non-dimensional time units. Therefore, we need to
run for a few of these convective units so that the fluid can "flush out" our initial
condition.

!alert warning
The number of convective units you need to reach stationarity depends on the problem
and you should remember to inspect *all* solution fields of interest. Depending on the
fluid properties and the initial conditions you chose, temperature could take longer
to reach stationarity than velocity, and vice versa.

#### Steady Flows

For laminar flows or for [!ac](RANS)
with steady boundary conditions, we expect to reach an actual steady state in the
solution fields. For these cases,
you can inspect the field files to get a sense of when this stationarity has been
reached, and you can also add C++ code to the `.udf` file to print out various
solution scalar quantities (e.g. maximum/minimum values, norms along lines, etc.).

Another easy way to monitor steady state is using Cardinal. For example, the input
below will run your NekRS case and automatically terminate once the relative change
in your solution $\vec{s}$ between two successive time steps $n$ and $n+1$ is less than a provided tolerance $\epsilon$.

\begin{equation}
\frac{\|\vec{s}^{n+1}-\vec{s}^n\|}{\Delta t\|\vec{s}^n\|}\leq\epsilon
\end{equation}

The solution $\vec{s}$ is the auxiliary system solution, meaning one long vector
containing all of the auxiliary variables (which you need to explicitly pass from NekRS's
internals into MOOSE variables using [FieldTransfers](FieldTransfers.md).
The norm above is scaled by $\Delta t$ so that if you have a very small time step,
the solution wouldn't change very much in such a short window of time (even if the
steady state has not been reached yet). You can also inspect postprocessors to see how
they vary during the simulation, for instance the bulk outlet temperature (which should
be $2H/R$ above the inlet bulk temperature (in non-dimensional terms) once reaching
the steady state.

!listing /tutorials/turbulence/inlet_outlet/nek.i

#### Turbulent Flows

For turbulent flow modeling using [!ac](LES) or [!ac](DNS), the flow will
reach statistical stationarity. Methods to evaluate this will be described later
in this tutorial.

## Periodic Boundary Conditions

For many
