This class must be used in conjunction with two other classes in Cardinal:

1. [NekRSMesh](NekRSMesh.md), which builds a mirror of the NekRS mesh
   in a MOOSE format so that all the usual
   [Transfers](Transfers/index.md)
   understand how to send data into/out of NekRS. The settings on
   [NekRSMesh](NekRSMesh.md) also determine which
   coupling type (listed above) are available.
2. [NekTimeStepper](NekTimeStepper.md), which allows NekRS to
   control its own time stepping.

Therefore, we recommend first reading the documentation for the above classes
before proceeding here.
