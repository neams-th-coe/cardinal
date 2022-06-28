This class must be used in conjunction with two other classes in Cardinal:

1. [NekRSMesh](/mesh/NekRSMesh.md), which builds a mirror of the NekRS mesh
   in a MOOSE format so that all the usual
   [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
   understand how to send data into/out of NekRS. The settings on
   [NekRSMesh](/mesh/NekRSMesh.md) also determine which
   coupling type (listed above) is used.
2. [NekTimeStepper](/timesteppers/NekTimeStepper.md), which allows NekRS to
   control its own time stepping.

Therefore, we recommend first reading the documentation for the above classes
before proceeding here.
