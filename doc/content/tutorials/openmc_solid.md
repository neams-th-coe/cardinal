# Temperature Coupling of OpenMC and MOOSE

These tutorials describe how to couple OpenMC to MOOSE for heat source
and temperature feedback. This is shown schematically in [cardinal_solid].

!media cardinal_solid.png
  id=cardinal_solid
  caption=Illustration of data transfers to couple OpenMC to MOOSE for temperature and heat source feedback.
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Cardinal uses a general formulation that allows OpenMC to couple via heat sources
and temperatures to *any* MOOSE application that can compute a temperature.
This is shown schematically in [cardinal_openmc_solid]. Pay special attention to the
"NekRS" bubble - OpenMC can couple via Cardinal to NekRS in the same manner
as any of these other codes.
All tutorials in this
section couple OpenMC to the MOOSE heat transfer module, but the concepts
extend equally to coupling OpenMC to any of these other MOOSE thermal-fluid codes.

!media cardinal_openmc_solid.png
  id=cardinal_openmc_solid
  caption=OpenMC integrates with *any* MOOSE application that can compute a temperature
  caption=Illustration of data transfers to couple OpenMC to MOOSE for temperature feedback.
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Several examples are provided:

- [LWR pincell](pincell1.md)
- [Solid UO$_2$ pebbles](triso.md)
- [TRISO compacts](gas_compact.md)
- [DAGMC pincell](dagmc.md)
- [DAGMC tokamak](tokamak.md)

Note that many of the features that are introduced in these
tutorials are *general* features that also apply to density feedback in OpenMC.
For instance, in the [pebble tutorial](triso.md), we describe how to
couple an OpenMC solve in centimeters with a MOOSE solve in meters.
Likewise, the [DAGMC pincell](dagmc.md) tutorial couples a [!ac](DAGMC) OpenMC
model to MOOSE, which is also applicable to DAGMC cells providing density feedback.
These
features are equally applicable to both temperature and density feedback in OpenMC,
even though the examples here only focus on temperature feedback. This allows
the tutorials to slowly progress in complexity.
