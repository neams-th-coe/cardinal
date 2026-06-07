# Temperature, Density, and Tally Coupling of OpenMC and MOOSE

These tutorials describe how to couple OpenMC to MOOSE via temperatures, densities,
and tallies (such as a fission power distribution).
This is shown schematically in [cardinal_solid].

!media cardinal_solid.png
  id=cardinal_solid
  caption=Illustration of data transfers to couple OpenMC to MOOSE for temperature, density, and tally feedback.
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Cardinal uses a general formulation that allows OpenMC to couple via tallies,
temperatures, and densities to *any* MOOSE application.
This is shown schematically in [cardinal_openmc_solid]. Pay special attention to the
"NekRS" bubble - OpenMC can couple via Cardinal to NekRS in the same manner
as any of these other codes.
All tutorials in this
section couple OpenMC to various MOOSE modules, but the concepts
extend equally to coupling OpenMC to any of these other MOOSE thermal-fluid codes.

!media cardinal_openmc_solid.png
  id=cardinal_openmc_solid
  caption=OpenMC integrates with *any* MOOSE application that can compute temperatures/densities or can consume a tally
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Several examples are provided:

- [LWR pincell](pincell1.md)
- [Solid UO$_2$ pebbles](triso.md)
- [TRISO compacts](gas_compact.md)
- [TRISO fuel assembly](openmc_fluid.md)
- [SFR fuel assembly](subchannel.md)
- [DAGMC pincell](dagmc.md)
- [DAGMC tokamak](tokamak.md)
