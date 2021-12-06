This directory contains a simulation of two concentric cylinders, where the
inner one is solved by MOOSE and the outer one is solved by nekRS. For comparison of the
heat conduction physics, the `moose.i` input file models the entire domain with the MOOSE heat
conduction module.

This case is also used to test a number of different run strategies, ensuring that the
same solution is obtained regardless of:

- whether nekRS uses a smaller time step than MOOSE (tested with `nek_master_subcycle.i`)
- whether nekRS runs first (tested with `nek_master_reversed.i`) or second
  (tested with `nek_master.i`) relative to MOOSE, when nekRS is the sub-app
- whether minimized data transfers in both the incoming and outgoing directions are
  used for nekRS (tested with `nek_master_mini.i` and `nek_mini.i`)
