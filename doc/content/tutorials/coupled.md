# Multiphysics Coupling of NekRS, OpenMC, and MOOSE

This tutorial describes how to combine NekRS, OpenMC, and MOOSE for
tightly-coupled multiphysics simulations.

[cardinal_multiapp2] illustrates how NekRS and OpenMC interact with MOOSE -
at a high level, "OpenMC talks to MOOSE" and "NekRS talks to MOOSE," such that
any simulation coupling OpenMC to NekRS is actually just communicating through a
MOOSE "intermediary." So, while all these tutorials here couple NekRS, OpenMC,
and the MOOSE heat conduction module, you can interchange the MOOSE heat conduction
module with any other MOOSE application that can compute solid temperature and
a heat flux.

!media framework_solid.png
  id=cardinal_multiapp2
  caption=High-level illustration of how NekRS and OpenMC couple to MOOSE
  style=width:90%;margin-left:auto;margin-right:auto;halign:center

Two examples are provided:

- [TRISO gas-cooled compact](triso_multiphysics.md)
- [SFR pincell](pincell_multiphysics.md)
- [MSFR](msfr.md)
