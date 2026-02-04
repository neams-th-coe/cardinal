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
  caption=NekRS flow domain. The inlet is located at 0 and the outlet is at 10.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

A computational mesh is built using MOOSE's mesh generators, shown
below. To generate this mesh, run

```
cardinal-opt -i pipe.i --mesh-only
mv pipe_in.e pipe.exo
```

!listing /tutorials/turbulence/pipe.i

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

From Paraview, we can also extract solution data along lines; for instance,
below shows the fluid pressure along the pipe axis. Our mesh is a bit too coarse
at the inlet, so there's some bumpiness there in pressure and velocity.
The fully-developed pressure gradient, taken by computing the difference in pressure
between two points near the outlet,

\begin{equation}
\frac{\pl P}{\pl z}_\text{fully developed}\approx\frac{P(0, 0, 10)-P(0, 0, 9.5)}{0.5}
\end{equation}

is around -0.3198 (nondimensional).

!media pressure_gradient.png
  id=pressure_gradient
  caption=Fluid pressure plotted down the center of the pipe
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

For many flows of interest, we are only interested in the fully-developed solution.
We can use inlet-outlet conditions for these scenarios, provided our domain is sufficiently
long enough and we only use the solution towards the outlet (once reaching fully-developed
conditions). However, this can result in a very long flow domain. For instance, turbulent
flow in a round pipe requires a development length $l$ of around

\begin{equation}
l=4.4Re^{1/6}D
\end{equation}

which can be 20-30 pipe diameters. For laminar flow, because the turbulent transport is
not aiding in the lateral transport across the pipe, the development length
can be even higher (on the order of 100 pipe diameters at the upper range of the laminar
regime). Therefore, if our interest is only in the fully-developed state, a natural
choice would be to instead model the pipe with periodic boundary conditions.

The case files for this periodic case are in the `periodic` subdirectory.

### Setting up a Periodic Mesh

To establish a periodic mesh, our 3-D fluid mesh must have two faces which are periodic
(identical topology). The element IDs on those faces must be shifted by an equal shift.
For instance, if one mesh face has three elements with IDs 1, 2, 3 then the corresponding
paired periodic mesh face would need to have IDs 9, 10, 11 (an equal shift of 8 for each).
You can easily attain this by generating a 2-D mesh of your geometry, and then extruding
it. This is how we made the exodus mesh for our pipe. However, we cannot simply
use the `pipe.re2` we generated for the inlet-outlet case earlier. We need to re-run
`exo2nek` and inform NekRS that we have one periodic pair of boundaries, and that those
boundary IDs are 2 and 3.

```
exo2nek
```

Now, we have a new `pipe.re2` file. For periodic cases, we need to have a `.usr`
file to (i) set the boundary IDs of any sidesets which are to be periodic, to zero
and (ii) renormalize any remaining boundary IDs so that they are sequential beginning
at 1.

!listing /tutorials/turbulence/periodic/pipe.usr

Then, in our `.par` file, we will only refer to the non-periodic boundaries which remain
(so, our `boundaryTypeMap` fields will only list the boundary condition for the solid
walls since that is the only boundary remaining in our mesh).

!listing /tutorials/turbulence/periodic/pipe.par

Note that in our `.oudf` file, we also only need to prescribe boundary conditions
for the non-periodic boundaries.

!listing /turbulence/periodic/pipe.oudf

### Periodic Flow and Temperature

NekRS treats the [!ac](GLL) points and their corresponding [!ac](DOFs) no different
than interior nodes. That is, the velocity, pressure, temperature, etc. at the nodes
on one periodic face are identical to those on the corresponding periodic face.

For the inlet/outlet case, the pressure gradient $\partial P/\partial z$ becomes constant in the fully-developed
region. Similarly, for a constant heat flux, the temperature gradient $\partial T/\partial z$ also becomes constant. Physically, this means that for the fully-developed flow
that the pressure and temperature will have a constant streamwise gradient - so by
definition, it's not possible for the pressure field to be identical on the inlet
and outlet face (and neither for temperature). Therefore, what NekRS actually solves
for in periodic flow cases is a decomposed pressure $\tilde{P}$ and a decomposed temperature $\tilde{T}$.

With a constant heat flux, we know that the temperature at a height $z$ will just
be shifted by some amount $\gamma z$ relative to the temperature at height 0.

\begin{equation}
T^\dagger(x,y,z,t)=\tilde{T}^\dagger(x,y,0,t)+\gamma z^\dagger
\end{equation}

From energy conservation, $\gamma$ is

\begin{equation}
\label{eq:gamma}
\begin{aligned}
q''^\dagger 2\pi R^\dagger z^\dagger=&\ \rho \pi (R^\dagger)^2U^\daggerC_p\underbrace{\left(T^\dagger(x,y,z,t)-\tilde{T}^\dagger(x,y,0,t)\right)}_{\gamma z^\dagger}\\
\frac{q''^\dagger 2}{\rho R^\dagger U^\dagger C_p} =&\ \gamma
\end{aligned}
\end{equation}

For this example, our heat flux is 1.0, and our nondimensional density, velocity, and specific
heat are also 1.0. Therefore, $\gamma=2/R$.

Now, we want to recast the energy equation in a way such that the outlet temperature does indeed equal the inlet temperature (so that our periodic boundary conditions work). Inserting the above into the conservation of energy equation,

\beq
\label{eq:c3}
\frac{\pl T}{\pl t}+V_i\frac{\pl T}{\pl x_i}=&\ \frac{k}{\rho C_p}\frac{\pl}{\pl x_i}\frac{\pl T}{\pl x_i}\\
\frac{\pl \tilde{T}}{\pl t}+V_i\frac{\pl\tilde{T}}{\pl x_i}=&\ \frac{k}{\rho C_p}\frac{\pl}{\pl x_i}\frac{\pl\tilde{T}}{\pl x_i}-V_z\gamma\\
\eeq

In this way, by adding a heat sink term $V_z\gamma$, the actual quantity we are solving for with the energy equation is the periodic temperature field $\tilde{T}$. This field can be a function of height if the geometry itself varies with $z$, such as in
wire-wrapped pin bundles (e.g. see [!cite](dutra)).
In other words, the representation in Eq. \eqref{eq:c2} does allow the periodic temperature field to vary in $z$ (i.e. if you were to plot $\tilde{T}$ along a vertical line, you would not see a constant temperature) unless your geometry had no change in the cross-sectional
geometry with height (like is the case for our simple pipe).

We need to add this heat sink, $\gamma z$, to our problem ourselves. This will require
adding a kernel to the energy equation. Our resulting temperature field that we compute will represent the fully-developed temperature, but scaled so that its average is zero.
Although including this heat sink is all that is strictly necessary, it is a good idea
to also explicitly subtract out any numerical drift in the temperature average.
Over long integration times, even if the bulk average is still a small number (e.g. $10^{-3}$), this can slowly drift over time.

!listing /tutorials/turbulence/periodic/pipe.udf

!listing /tutorials/turbulence/periodic/pipe.oudf

Likewise for pressure, NekRS will solve for the pressure field superimposed on top of
the constant-pressure-gradient arising from fully-developed flow.

\begin{equation}
\label{eq:lambda}
P(x,y,z,t)=\tilde{P}(x,y,0,t)-\lambda z
\end{equation}

To correctly set up the flow aspects of a periodic case, all we need to add is the
following line to the `.par` file, where the `meanVelocity` should be the mean
velocity (for our non-dimensional case, this is the mean value of $U^\dagger$ which is
just 1 because we choose a reference velocity scale $U_0$ to be the mean dimensional axial
velocity). The `direction` indicates which coordinate direction represents the periodic
direction.

```
constFlowRate = meanVelocity=1.0 + direction=Z
```

For the round pipe, our pressure distribution we will solve for with NekRS is
simply zero (because our true pressure distribution is simply a constant axial gradient).
Whereas in Eq. \eqerf{eq:gamma} we know how to scale back from our non-dimensional
fully-developed temperature, you don't need to manually make any changes to kernels/etc.
to accomplish the periodic flow (aside from setting the `constFlowRate`).
When running the case, NekRS will print to the screen the
fully developed pressure gradient ($\lambda$)
in Eq. \eqref{eq:lambda} as the `scale` term. For instance, for the time step shown
below, the fully developed pressure gradient is 3.0882e-1 (compare this to
the value we estimated from our inlet/outlet case earlier).

```
Time Step 623, time = 11.0732, dt = 0.016456
copying solution to nek
S00      : iter 011  resNorm0 4.63e-07  resNorm 8.63e-08
projP    : resNorm0 1.56e-07  resNorm 9.08e-12  ratio = 1.721e+04  5/8
P        : iter 001  resNorm0 9.08e-12  resNorm 1.46e-12
UVW      : iter 035  resNorm0 2.21e-03  resNorm 7.00e-08  divErrNorms 3.23e-14 3.00e-07
flowRate : uBulk0 9.97e-01  uBulk 1.00e+00  err 2.22e-16  scale 3.08822e-01
step= 623  t= 1.10731611e+01  dt=1.6e-02  C= 0.48  elapsedStep= 5.07e-02s  elapsedStepSum= 3.55035e+01s
```

We are now ready to run this case. We will use the Cardinal input from the inlet/outlet
case earlier to easily monitor steady state detection. To run with 12 ranks
(just an example),

```
mpiexec -np 12 cardinal-opt -i nek.i
```

You will notice that with the periodic case, we reach a steady-state much faster
than the inlet/outlet case. Pressure is effectively zero. Shown below is the
temperature, $\tilde{T}$.

!media pipe_periodic.png
  id=pipe
  caption=NekRS temperature solution for periodic boundary conditions.
  style=width:60%;margin-left:auto;margin-right:auto;halign:center
