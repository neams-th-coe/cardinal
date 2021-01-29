This directory contains a simulation of two rectangular prisms adjacent to one another, where the
left side is solved by MOOSE and the right side solved by nekRS. For comparison of the
heat conduction physics, the `moose.i` input file models the entire domain with the MOOSE heat
conduction module.

In this example, the interface mesh between BISON/nekRS and MOOSE/MOOSE are exactly the
same - there is no curvature in the nekRS model, and the elements all line up one-to-one.
Therefore, this directory does _not_ test:

* Whether the flux interpolation from MOOSE to nekRS works correctly, since the constant
  monomial flux from MOOSE would always be reconstructed exactly on the nekRS side.
