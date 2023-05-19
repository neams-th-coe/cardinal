# Multiphysics Coupling of NekRS, OpenMC, and MOOSE

These tutorials describe how to combine NekRS, OpenMC, and MOOSE for
tightly-coupled multiphysics simulations.

[cardinal_multiapp2] illustrates how NekRS and OpenMC interact with MOOSE -
at a high level, "OpenMC talks to MOOSE" and "NekRS talks to MOOSE," such that
any simulation coupling OpenMC to NekRS is actually just communicating through a
MOOSE "intermediary." So, while all these tutorials here couple NekRS, OpenMC,
and the MOOSE heat conduction module, all three of these codes are completely
interchangeable with other MOOSE applications. For instance, OpenMC could be
swapped with Griffin with virtuall no changes to the MOOSE and NekRS input files.

!media framework_solid.png
  id=cardinal_multiapp2
  caption=High-level illustration of how NekRS and OpenMC couple to MOOSE
  style=width:90%;margin-left:auto;margin-right:auto;halign:center

Several examples are provided:

- [TRISO gas-cooled compact](triso_multiphysics.md)
- [SFR pincell](pincell_multiphysics.md)
- [MSFR](msfr.md)
