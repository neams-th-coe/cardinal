# Conjugate Heat Transfer Coupling of NekRS and MOOSE

These tutorials describe how to couple NekRS to MOOSE through *boundary*
heat transfer by exchanging boundary conditions for wall temperature and heat flux.
This is shown schematically in [sfr_cht].
These types of calculations are referred to as *conjugate heat transfer*.

!media cht.png
  id=sfr_cht
  caption=Illustration of data transfers to couple NekRS to MOOSE for boundary heat transfer in a pin bundle.
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Cardinal uses a general formulation that allows NekRS to couple via conjugate
heat transfer to *any* MOOSE application that can compute a heat flux. This
is shown schematically in [cardinal_cht]. All tutorials in this section couple
NekRS to the MOOSE heat conduction module, but the concepts extend equally to
coupling NekRS to any of these other MOOSE thermal-fluid codes.

!media cardinal_cht.png
  id=cardinal_cht
  caption=NekRS integrates with *any* MOOSE application that can compute a heat flux
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Four examples are provided:

- [Reflector Bypass Flow](cht1.md) around a reflector block in a pebble bed reactor
- [Pin Bundle Flow](cht2.md) in a bare 7-pin geometry
- [Turbulent RANS Channel Flow](cht3.md) in a heated [!ac](TRISO) compact
- [Turbulent LES Flow](cht4.md) in a heated 67-pebble [!ac](HTGR)
