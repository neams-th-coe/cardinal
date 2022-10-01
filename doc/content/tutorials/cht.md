# Conjugate Heat Transfer Coupling of NekRS and MOOSE

These tutorials describe describe how to couple NekRS to MOOSE through *boundary*
heat transfer by exchanging boundary conditions for wall temperature and heat flux.
This is shown schematically in [sfr_cht].
These types of calculations are referred to as *conjugate heat transfer*.

!media cht.png
  id=sfr_cht
  caption=Illustration of data transfers to couple NekRS to MOOSE for boundary heat transfer in a pin bundle.
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Three examples are provided:

- [Reflector Bypass Flow](cht1.md) around a reflector block in a pebble bed reactor
- [Pin Bundle Flow](cht2.md) in a bare 7-pin geometry
- [Turbulent Channel Flow](cht3.md) in a [!ac](TRISO) compact
