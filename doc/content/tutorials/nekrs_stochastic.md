# Stochastic Simulations with NekRS

In this tutorial, you will learn how to:

- Stochastically perturb multiphysics NekRS simulations for forward [!ac](UQ)
  using the MOOSE [Stochastic Tools Module](https://mooseframework.inl.gov/modules/stochastic_tools/).

The MOOSE [!ac](STM)
is an open-source module in the MOOSE framework for seamlessly integrating
massively-parallel [!ac](UQ), surrogate modeling, and machine learning with
MOOSE-based simulations. Here, we will perform a "parameter study" of a
conjugate heat transfer coupling of MOOSE heat conduction and NekRS. We will
stochastically sample a parameter (the thermal conductivity in the NekRS model)
from a distribution $N$ times, each time re-running the multiphysics simulation.
For each independent run, we will compute a [!ac](QOI) (maximum NekRS temperature)
and evaluate various statistical quantities for that [!ac](QOI)
(e.g. mean, standard deviation, confidence intervals).

Cardinal's interface between NekRS and the [!ac](STM) is designed in an
infinitely-flexible manner. *Any* quantity in NekRS which can be modified from
the `.udf` and `.oudf` files can be stochastically perturbed by MOOSE. In this example,
we will just perturb one quantity (thermal conductivity), though you can simultaneously
perturb an arbitrary number of parameters. Other notable features include:

- You can perturb parameters nested arbitrarily "beneath" the driver stochastic
  application. We will use this to send a thermal conductivity value
  from the [!ac](STM), to a solid heat conduction sub-app, which then passes the
  value to a NekRS sub-sub app.
- You can use a single perturbed value in multiple locations in dependent applications.
  For example, if you use a multiscale fluid model with NekRS in one part of the domain
  and the MOOSE [Navier-Stokes module](https://mooseframework.inl.gov/modules/navier_stokes/index.html)
  in another region of space, you could send a single perturbed value for fluid density
  to both applications concurrently.

Before reading this tutorial, we recommend reading through [Examples 1 through 3](https://mooseframework.inl.gov/modules/stochastic_tools/)
 on the [!ac](STM) page to familiarize yourself with the basic syntax for stochastic simulations.

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
!alert-end!

To access this tutorial,

```
cd cardinal/tutorials/nek_stochastic
```

## Problem Setup

In this problem, we couple a MOOSE solid heat conduction model with a NekRS
heat conduction model in two adjacent domains, shown in [slab_model]. A uniform heat
flux of 1000 W/m$^2$ is applied on the left face of the MOOSE region, whereas a fixed
Dirichlet temperature of 500 K is applied on the right face of the NekRS region.
MOOSE and NekRS are coupled via heat flux and temperature on the interface. MOOSE will
send a heat flux boundary condition to NekRS, while NekRS will return an interface
temperature to MOOSE for use as a Dirichlet condition. All other surfaces are insulated.

!media slab_model.png
  id=slab_model
  caption=Two-region heat conduction model used for demonstrating stochastic simulations
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

We choose this problem for demonstration because the steady-state solution has a simple
analytic form. The general shape for the temperature distribution is shown as the orange
line in the right of [slab_model],
where the slopes of the temperature in the two blocks are governed by their respective
thermal conductivities. The temperature at the interface, $T_i$, between MOOSE and
NekRS is governed by

\begin{equation}
\label{analytic_ti}
q^{''}=k_n\frac{T_i-500}{L_{n}}
\end{equation}

where $q^{''}$ is the imposed heat flux on the solid model, $k_n$ is the thermal conductivity
of the NekRS domain, $L_n$ is the width of the NekRS region. Similarly, the temperature at
the left face, $T_{l}$, is governed by

\begin{equation}
q^{''}=k_m\frac{T_l-T_i}{L_{m}}
\end{equation}

where $k_m$ is the thermal conductivity of the MOOSE domain and $L_m$ is the width of the MOOSE region.
We will stochastically perturb $k_n$, the thermal conductivity of the NekRS domain,
by sampling it from a normal distribution with mean value of 5.0 and standard deviation
of 1.0. For each choice of $k_n$, we will run this coupled heat transfer model, and
predict $T_i$ as our [!ac](QOI).

## NekRS Model

First, we need to build a mesh for NekRS. We use MOOSE mesh generators to build the
rectangular prism with $L_n=1$. Because NekRS's sidesets must be numbered contiguously
beginning from 1, we need to shift the default sideset IDs created by the
[GeneratedMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/GeneratedMeshGenerator.html).

!listing /tutorials/nek_stochastic/channel.i
  block=Mesh

We can generate an Exodus mesh by running in `--mesh-only` mode:

```
cardinal-opt -i channel.i --mesh-only
mv channel_in.e channel.exo
```

Then, we convert into NekRS's custom `.re2` mesh format by running the `exo2nek`
script. Alternatively, you can simply use the `channel.re2` file already checked
into the repository.
The remaining NekRS input files are:

- `channel.par`: High-level settings for the solver, boundary
  condition mappings to sidesets, and the equations to solve
- `channel.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `channel.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and
  source terms

In `channel.par`, we specify that we want to solve for temperature while disabling
the fluid solve by setting `solver = none` in the `[VELOCITY]` block. The default
initial condition for velocity is a zero value, so this input file is essentially
solving for solid heat conduction,

\begin{equation}
\rho C_p\frac{\partial T}{\partial t}-\nabla\cdot\left(k_n\nabla T\right)=0
\end{equation}

where $\rho C_p=50$ and $k_n$ will be specified by MOOSE. Normally, all that
you need to do to set thermal conductivity in NekRS is to set the
`conductivity` parameter in the `[TEMPERATURE]` block. Here, we will instead
retrieve this value from MOOSE through the `.udf`.

!alert warning
However, the user must still set `conductivity` in the `.par` file because
this value is used to initialize the elliptic solvers in NekRS. Do not
simply put an arbitrary value here, because an unrealistic value will hamper
the convergence of the elliptic solvers for the entire NekRS simulation.
In order to initialize the elliptic solvers and preconditioners, we recommend
setting the properties in the `.par` file to their mean values.

!listing /tutorials/nek_stochastic/channel.par

Next, in the `channel.oudf` we specify our boundary conditions in the NekRS model --
a Dirichlet temperature of 500 on sideset 3 and a heat flux from MOOSE on
sideset 5.

!listing /tutorials/nek_stochastic/channel.oudf language=cpp

Finally, the `channel.udf` file contains user-defined functions.
Here, we are going to use the `udf.properties` to apply custom material properties.
In the `uservp` function, we are reading thermal conductivity from the scratch space
`nrs->usrwrk` (to be described in more detail soon). This scratch space is where MOOSE
writes *any* data going into NekRS -- which we have used extensively so far to send
heat flux, volumetric heating, etc. from MOOSE to NekRS. Now, we will use this same
scratch space to fetch a stochastically sampled value for thermal conductivity.

This value is then passed into the thermal conductivity array, slot 0 in `o_SProp`,
by calling the `platform->linAlg->fill` function. If you wanted to stochastically
perturb other quantities, like the fluid viscosity, you would access those in a different
way. For a summary of how to access other quantities in NekRS, we will provide a table
at the end of this tutorial.

!listing /tutorials/nek_stochastic/channel.udf language=cpp

In order to ensure that the most up-to-date stochastic data from MOOSE is used
in each scalar and flow solver, we add an extra call to `udf.properties` in Cardinal
*each time* MOOSE sends new data to NekRS. So,
`udf.properties` is called:

1. Once before time stepping begins (within `nekrs::setup`). At this point,
  `nrs->usrwrk` does not exist yet, which is why we return without doing anything
  if `nrs->usrwrk` is a null pointer (the `if (!nrs->usrwrk)` line). This means that
  when the elliptic solvers are initialized, they are using whatever property values
  are set in the `.par` file (as already described).
2. (*special to Cardinal*) Immediately after MOOSE sends new stochastic data to NekRS
3. Between each scalar solve and the ensuing flow solve

Steps 1 and 3 already occur in standalone NekRS simulations, so Cardinal is just adding
an extra call to ensure consistency.

The last thing we need is the thin wrapper input file which runs NekRS.
We start by building a mesh mirror of the NekRS mesh, as a
[NekRSMesh](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html). We will couple
via conjugate heat transfer through boundary 5 in the NekRS domain. Next, we use
[NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html)
to specify that we will replace MOOSE solves with NekRS solves. The
[NekTimeStepper](https://cardinal.cels.anl.gov/source/timesteppers/NekTimeStepper.html)
finally will then allow NekRS to control its time stepping, aside from synchronization
points imposed by the main app.

!listing /tutorials/nek_stochastic/nek.i
  end=UserObjects

Next, we add a [NekScalarValue](https://cardinal.cels.anl.gov/source/userobjects/NekScalarValue.html)
to be the receiving point for a stochastic number coming from "higher up" in the multiapp
hierarchy (to be described soon). Then, we define a number of postprocessors. The `max_temp`
postprocessor will be our [!ac](QOI) evaluating $T_i$ which will get passed upwards in the multiapp
hierarchy. The [NekScalarValuePostprocessor](https://cardinal.cels.anl.gov/source/postprocessors/NekScalarValuePostprocessor.html)
is added for convenience to display the value held by the `NekScalarValue` userobject to the console
so that we can easily see each random value for $k_n$ getting received. For sanity check, we also
add `expect_max_T` to compute the analytic value for $T_i$ according to [analytic_ti].

!listing /tutorials/nek_stochastic/nek.i
  start=UserObjects
  end=Controls

Lastly, we need to have a [SamplerReceiver](https://mooseframework.inl.gov/source/controls/SamplerReceiver.html), which will actually facilitate receiving the stochastic data into the
[NekScalarValue](https://cardinal.cels.anl.gov/source/userobjects/NekScalarValue.html)
object. *Any* input file receiving stochastic data just needs to have one of these objects.

!listing /tutorials/nek_stochastic/nek.i
  block=Controls

## Heat Conduction Model

For the MOOSE heat conduction model, we firt set up a
mesh and create the nonlinear variables, kernels, and boundary conditions to solve
the solid heat conduction equation. The `flux` auxiliary variable will be used to
compute the interface heat flux (which will get sent to NekRS), while the `nek_temp`
auxiliary variable will be a receiver for interface temperature from NekRS.
NekRS must run using a transient executioner, so we will use the `steady_state_detection`
feature here to ensure that each NekRS simulation has reached the pseudo-steady state.

!listing /tutorials/nek_stochastic/ht.i
  end=MultiApps

Next, we add NekRS as a sub-app, and define the transfers. The `temperature`, `flux`,
and `flux_integral` transfers we have seen many times before -- these are transferring
data necessary to apply the heat flux and temperature coupled boundary conditions on the
interfaces. The [MultiAppReporterTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppReporterTransfer.html)
is used to fetch the value held by the `max_temp` postprocessor in the `nek.i` input file.
A Reporter is basically a vector-valued postprocessor which could be used to hold multiple
[!ac](QOI)s. In this example, we are just interested in one value.

!listing /tutorials/nek_stochastic/ht.i
  start=MultiApps
  end=Reporters

Finally, we declare a [ConstantReporter](https://mooseframework.inl.gov/source/reporters/ConstantReporter.html)
to be the receiving point of the data fetched by the `get_qoi` transfer. Note that we do
*not* need a [SamplerReceiver](https://mooseframework.inl.gov/source/controls/SamplerReceiver.html)
in this input file because the MOOSE heat conduction model itself is not receiving any
stochastic inputs - the NekRS thermal conductivity passes straight from the [!ac](STM)
to NekRS.

!listing /tutorials/nek_stochastic/ht.i
  block=Reporters

## Stochastic Tools Module

For stochastic simulations, a [!ac](STM) input "drives" the entire simulation as the
main app. First, we define random distributions and sampling methods for our random
data. Here, we will use a normal distribution for $k_n$. We choose a
[MonteCarlo](https://mooseframework.inl.gov/source/samplers/MonteCarloSampler.html) sampler
to randomly sample from this normal distribution. Here, `num_rows` is the number
of independent samples to take.

!listing /tutorials/nek_stochastic/driver.i
  end=MultiApps

Next, we add the MOOSE heat conduction model as a [SamplerFullSolveMultiApp](https://mooseframework.inl.gov/source/multiapps/SamplerFullSolveMultiApp.html),
which will launch one new physics simulation for each random sample.
We next use a [SamplerParameterTransfer](https://mooseframework.inl.gov/source/transfers/SamplerParameterTransfer.html)
to send the random numbers from the `sample` object into the `k` UserObject
in the sub-sub app named `nek`. In order to fetch the [!ac](QOI) from the
nested physics apps, we use a [SamplerReporterTransfer](https://mooseframework.inl.gov/source/transfers/SamplerReporterTransfer.html)
to get the $T_i$ from the sub-app. This will be held by the `storage` [StochasticMatrix](https://mooseframework.inl.gov/source/reporters/StochasticMatrix.html). Finally, we
use a [StatisticsReporter](https://mooseframework.inl.gov/source/reporters/StatisticsReporter.html)
to conduct some statistical analysis on our [!ac](QOI).

!listing /tutorials/nek_stochastic/driver.i
  start=MultiApps

## Summary

[summary] summarizes the data flow for this stochastic simulation. The only specialization for
NekRS is the use of [NekScalarValue](https://cardinal.cels.anl.gov/source/userobjects/NekScalarValue.html)
to receive the data, which the user is then responsible for applying in the NekRS input files
as appropriate.

!media summary_st.png
  id=summary
  caption=Summary of major data transfers and objects used to accomplish stochastic simulations with NekRS
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

## Execution and Postprocessing

To run the stochastic simulation

```
mpiexec -np 2 cardinal-opt -i driver.i
```

When you first run this input file, you will see a table print at the beginning of the NekRS
solve which shows:

```
  ===================>     MAPPING FROM MOOSE TO NEKRS      <===================

           Slice:  entry in NekRS scratch space
        Quantity:  physical meaning or name of data in this slice
   How to Access:  C++ code to use in NekRS files; for the .udf instructions,
                   'n' indicates a loop variable over GLL points

 ------------------------------------------------------------------------------------------------
 | Quantity |            How to Access (.oudf)          |         How to Access (.udf)          |
 ------------------------------------------------------------------------------------------------
 | flux     | bc->usrwrk[0 * bc->fieldOffset + bc->idM] | nrs->usrwrk[0 * nrs->fieldOffset + n] |
 | k        | bc->usrwrk[1 * bc->fieldOffset + 0]       | nrs->usrwrk[1 * nrs->fieldOffset + 0] |
 | unused   | bc->usrwrk[2 * bc->fieldOffset + bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[3 * bc->fieldOffset + bc->idM] | nrs->usrwrk[3 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[4 * bc->fieldOffset + bc->idM] | nrs->usrwrk[4 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[5 * bc->fieldOffset + bc->idM] | nrs->usrwrk[5 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[6 * bc->fieldOffset + bc->idM] | nrs->usrwrk[6 * nrs->fieldOffset + n] |
 ------------------------------------------------------------------------------------------------
```

This table will show you how the [NekScalarValues](https://cardinal.cels.anl.gov/source/userobjects/NekScalarValue.html)
map to particular locations in the scratch space. It is from this table that we knew the
value of thermal conductivity would be located at `nrs->usrwrk[1 * nrs->fieldOffset + 0]`,
which is what we used in the `channel.udf`.

!alert note
When you run this input file,
the screen output from NekRS itself is very messy, because NekRS will output any print
statements which occur on rank 0 of the local communicator. For running stochastic simulations
via MOOSE, we split a starting communicator into smaller pieces, which means that multiple NekRS
cases are all trying to print to the console at the same time, clobbering each other. We are
working on a better solution.

Because we specified JSON output in the `driver.i`, this will create a number of output files:

- `driver_out.json`, which contains the stochastic results for [!ac](QOI)s. We used
  `parallel_type = ROOT` in the [StochasticMatrix](https://mooseframework.inl.gov/source/reporters/StochasticMatrix.html), which will collect all the stochastic data into one file at the end of
  the simulation. If you had omitted this value, you would end up with $n$ JSON output
  files, where $n$ is the number of processors.
- `driver_out_ht<n>_nek0.csv`, CSV files with the NekRS postprocessors, where `<n>` is
  each independent coupled simulation (i.e. each represents a conjugate heat transfer
  simulation with a different value for $k_n$).

We can use Python scripts in the MOOSE stochastic tools module to plot our
[!ac](QOI) as a histogram. Using the `make_histogram.py` script, we can indicate
that we want to plot the `results:receive:nek_max_T` data we fetched with the
[StochasticMatrix](https://mooseframework.inl.gov/source/reporters/StochasticMatrix.html),
and save the histogram in a file named `Ti.pdf`, shown in [Ti_img] for 200 samples.

```
python ../../contrib/moose/modules/stochastic_tools/python/make_histogram.py driver_out.json* -v results:receive:nek_max_T --xlabel 'Interface Temperature' --output Ti.pdf
```

!media Ti.png
  id=Ti_img
  caption=Histogram of $T_i$ when running with `num_rows = 200` samples
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

We can also directly plot the random values sampled, i.e. the input distribution. These
are shown in [samples].

```
python ../../contrib/moose/modules/stochastic_tools/python/make_histogram.py driver_out.json* -v sample_0 --xlabel 'Input Distribution' --output samples.pdf
```

!media samples.png
  id=samples
  caption=Actual sampled input distribution for $k_n$
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

Because we added the the [StatisticsReporter](https://mooseframework.inl.gov/source/reporters/StatisticsReporter.html),
we also have access to the mean, standard deviation, and confidence intervals automatically
(of course, you could post-compute those as well). We can output this
data as a table, using the `visualize_statistics.py` script.

```
python ../../contrib/moose/modules/stochastic_tools/python/visualize_statistics.py driver_out.json --markdown-table --names '{"storage_results:receive:nek_max_T":"Interface Temperature"}'
```

which will output

```
| Values                                 | MEAN                 | STDDEV              |
|:---------------------------------------|:---------------------|:--------------------|
| Interface Temperature (5.0%, 95.0%) CI | 714.4 (709.3, 719.8) | 45.1 (39.98, 50.22) |
```

We invite you to explore these plotting scripts and documentation in the [!ac](STM),
as this tutorial only scratches the surface with the simulations performed. You can make bar plots
and many other data presentation formats.

## Other Features

By default, when you run NekRS in a stochastic simulation, each individual run will
write the NekRS solution to the same field files -- `<case>0.f*`. These will all clobber eachother.
If you want to preserve
a unique output file for each of the NekRS runs, you can set `write_fld_files = true`
in the `[Problem]` block, which will write output files labeled as `a00case`, `a01case`, ...,
`z98case`, `z99case`, giving you 2574 possible output files.

## Perturbing NekRS Parameters

In this example, we perturbed the thermal conductivity, but there are many other parameters
which can be modified in NekRS via the `udf.properties` function.
[perturbed_params] summarizes the parameters which can be modified by the `udf.properties` function.
Because NekRS simulations can
be run with any number of passive scalars, the content and ordering will depend on how many
scalars you have. So, to be as general as possible, we've written [perturbed_params]
assuming you have a simulation with 3 passive scalars.

!table id=perturbed_params caption=Commmon parameters to modify in NekRS via `udf.properties`
| Parameter | How to Access |
| :- | :- |
| Coefficient on time derivative of 0th scalar equation | 0th slot in `o_SProp` |
| Coefficient on time derivative of 1st scalar equation | 1st slot in `o_SProp` |
| Coefficient on time derivative of 2nd scalar equation | 2nd slot in `o_SProp` |
| Coefficient on diffusion operator of 0th scalar equation | 3rd slot in `o_SProp` |
| Coefficient on diffusion operator of 1st scalar equation | 4th slot in `o_SProp` |
| Coefficient on diffusion operator of 2nd scalar equation | 5th slot in `o_SProp` |
| Fluid dynamic viscosity | 0th slot in `o_UProp` |
| Fluid density | 1st slot in `o_UProp` |

Other quantities which you can perturb include:

- Boundary conditions
- Anything in a GPU kernel, such as momentum or heat source terms
- Geometry
