This directory contains a simulation of two pyramidal prisms adjacent to one another, where
block 2 is solved by MOOSE and block 1 is solved by nekRS. For comparison of the
heat conduction physics, the `moose.i` input file models the entire domain with the MOOSE heat
conduction module.

In this example, the interface mesh between BISON/nekRS and MOOSE/MOOSE are exactly the
same - there is no curvature in the nekRS model, and the elements all line up one-to-one.
Therefore, this directory does _not_ test:

* Whether the flux interpolation from MOOSE to nekRS works correctly, since the constant
  monomial flux from MOOSE would always be reconstructed exactly on the nekRS side.

This simulation differs from the simpler case in the `cube` directory because the analytical
heat flux at the interface is _not_ constant over each element (but we represent it as a
constant monomial in the model). Therefore, we should expect slightly higher errors compared
to the standalone MOOSE simulation.
