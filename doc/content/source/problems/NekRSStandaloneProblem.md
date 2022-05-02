# NekRSStandaloneProblem

This class performs all activities related to solving NekRS as a MOOSE application.
This class differs from the [NekRSProblem](/problems/NekRSProblem.md) class because this class
does not *couple* NekRS to another MOOSE application. Instead, this class
simply allows standalone NekRS simulations to be run using the Cardinal executable,
which allows uncoupled NekRS simulations to take advantage of a number of features
available in Cardinal, such as:

- Postprocessors to query the solution, evaluate heat balances and pressure drops,
  or evaluate solution convergence
- UserObjects to project the NekRS solution onto other discretization schemes,
  such as a subchannel discretization, or onto other MOOSE applications, such as
  for providing closures
- Automatic conversion of nondimensional NekRS solutions into dimensional form

In other words, this class allows NekRS input files to be run through Cardinal
without *any* modifications.
This class must be used in conjunction with the
[NekRSMesh](/mesh/NekRSMesh.md) and the [NekTimeStepper](/timesteppers/NekTimeStepper.md).
Therefore, we first recommend reading the documentation for `NekRSMesh` and `NekTimeStepper`
before proceeding here.

The smallest possible MOOSE-wrapped input file that can be used to run NekRS standalone
cases is shown below. This input file in particular runs the `channel` example that ships
with the NekRS repository - if you navigate to the `test/tests/nek_standalone/channel` directory,
you will see that all the NekRS case files are the same as the examples that ship with NekRS.

In this input file, `casename` is the prefix describing the NekRS input files, i.e. this
parameter would be `casename = channel` if the NekRS input files are `channel.re2`,
`channel.par`, `channel.udf`, and `channel.oudf`. The `NekRSStandaloneProblem` class controls
execution of NekRS as if it were a MOOSE application, [NekRSMesh](/mesh/NekRSMesh.md) creates
a mesh mirror for visualization of the NekRS solution (in addition to the usual NekRS field
file output), and [NekTimeStepper](/timesteppers/NekTimeStepper.md) allows NekRS to control its
own time stepping.

!listing test/tests/nek_standalone/channel/nek.i
  end=Postprocessors

With this input, you can equivalently run the `channel` NekRS example either with
the standalone NekRS executable,

```
$ nrsmpi channel 8
```

or through the Cardinal executable,


```
$ mpiexec -np 8 cardinal-opt -i nek.i
```

Both will run NekRS in exactly the same fashion using exactly the same NekRS input files.
All that is needed to wrap NekRS in MOOSE is the thin input file shown previously.
The remainder of this page highlights a few potential use cases for running standalone
NekRS cases through Cardinal.

## Postprocessing the NekRS Solution

An advantage of running NekRS cases through Cardinal is the ability to leverage
a wide set of MOOSE postprocessors to both monitor the solution as the solve
progresses and to easily perform common postprocessing steps.
The example below runs the `ktauChannel` NekRS example through Cardinal.
The `max_T` postprocessor computes the maximum
temperature by directly querying the NekRS solution; many other available
postprocessors of this type are available in Cardinal and described
on the [source documentation page](https://cardinal.cels.anl.gov/source/index.html).

!listing test/tests/nek_standalone/ktauChannel/nek.i
  end=UserObjects

However, when specifying fields to output from the NekRS solution with
the `output` parameter (described in more detail in [#output]), any of the
wide set of [MOOSE postprocessors](https://mooseframework.inl.gov/source/index.html)
can also be used. For instance, a [PointValue](https://mooseframework.inl.gov/source/postprocessors/PointValue.html) postprocessor
is used to extract the value of temperature at a specific point in space,
and a [PercentChangePostprocessor](https://mooseframework.inl.gov/source/postprocessors/PercentChangePostprocessor.html)
is used to evaluate the percent change in the maximum temperature
between two successive time steps such as for assessing convergence.
A few examples of other postprocessors that may be of use to NekRS
simulations include:

- [ElementL2Error](https://mooseframework.inl.gov/source/postprocessors/ElementL2Error.html),
  which computes the L$^2$ norm of a MOOSE variable relative to a provided
  function, useful for solution verification
- [FindValueOnLine](https://mooseframework.inl.gov/source/postprocessors/FindValueOnLine.html),
  which finds the point at which a specified value of a variable occurs,
  which might be used for directly evaluating a boundary layer thickness
- [LinearCombinationPostprocessor](https://mooseframework.inl.gov/source/postprocessors/LinearCombinationPostprocessor.html),
  which can be used to combine any existing postprocessors together in a
  general expression $a_0p_0+a_1p_1+\cdots+b$, where $a_i$ are coefficients,
  $p_i$ are postprocessors, and $b$ is a constant additive factor. This can be used
  to compute the temperature *rise* in a domain by subtracting a postprocessor
  that computes the inlet temperature from a postprocessor that computes the
  outlet temperature.
- [TimeExtremeValue](https://mooseframework.inl.gov/source/postprocessors/TimeExtremeValue.html),
  which provides the maximum/minimum value of a variable over the course of
  an entire simulation, such as for evaluating peak stress in an
  oscillating system

Please consult the [MOOSE documentation](https://mooseframework.inl.gov/source/index.html)
for a full list of available postprocessors.

In addition to the postprocessor system, a wide set of userobjects
and auxiliary kernels may be applied to the NekRS solution. For instance,
a [LayeredAverage](https://mooseframework.inl.gov/source/userobject/LayeredAverage.html)
userobject can be used to compute an average of a MOOSE variable in a number
of layers. Here, this userobject is applied to average the pressure in four
layers in the $x$ direction (the direction parallel to the wall). The results
of this userobject are then visualized with a
[SpatialUserObjectAux](https://mooseframework.inl.gov/source/auxkernels/SpatialUserObjectAux.html)
to place the results in the `layered_p` auxiliary variable.

!listing test/tests/nek_standalone/ktauChannel/nek.i
  start=UserObjects

These two examples of postprocessor and userobject usage only scratch the
surface of the capabilities of the MOOSE framework - please consult
the [MOOSE documentation](https://mooseframework.inl.gov/source/index.html) to learn about further capabilities.

## Outputting the NekRS Solution
  id=output

!include output_solution.md

For example, if you would like to extract the NekRS solution on an Exodus mesh,
simply specify the `output` parameter with the desired fields. The following input
sections will interpolate the NekRS pressure, velocity, and temperature for
the `lowMach` NekRS example
onto a second order representation of the NekRS mesh.

!listing test/tests/nek_standalone/lowMach/nek.i
  end=Executioner

Then, the output format specified in the `[Output]` block will be used. Here,
the NekRS solution is projected onto an Exodus mesh.

!listing
[Outputs]
  exodus = true
[]

For instance, [output_p] shows the pressure from the NekRS field files (left)
and interpolated onto a second-order mesh mirror (right). Because this particular
example runs NekRS in a higher order than can be represented on a second-order
mesh mirror, the interpolated pressure is clearly not an exact
representation of the NekRS solution - only an interpolated version of the NekRS solution.

!media output_p.png
  id=output_p
  caption=Pressure from the NekRS field files (left) and after interpolation onto a second order mesh mirror (right).
  style=width:90%;margin-left:auto;margin-right:auto;halign:center
