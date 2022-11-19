This page describes how NekRS is executed within MOOSE.
The `externalSolve()` function runs a single NekRS time step. This involves:

- Advance one CFD time step by calling `nekrs::runStep`. This will print out something showing the results of the NekRS
   solve, like

```
  P        : iter 200  resNorm0 7.13e-04  resNorm 7.21e-05
  UVW      : iter 019  resNorm0 2.19e-02  resNorm 9.08e-07  divErrNorms 6.29e-03 6.60e-01
  S00      : iter 184  resNorm0 9.98e+01  resNorm 8.73e-04
step= 6  t= 3.0e-02  dt=5.0e-03  C= 0.31  UVW: 19  P: 20  S: 184  elapsedStep= 2.21e+00s  elapsedStepSum= 1.33800e+01s
```

- Optionally limit the temperature on the host. This will print out something like the following, after which we copy from the host to the device.

```
Limiting nekRS temperature to within the range 0.0, 1.0
```

- Copy the NekRS solution to the Nek5000 backend so that it is accessible for writing field files, interacting with via the legacy `.usr`-style approach, etc. At this point, the NekRS solution has been copied from device to host, so you can access it host-side.
- Write field files.
