# Conjugate Heat Transfer Coupling of NekRS and MOOSE

These tutorials describe how to couple NekRS to MOOSE through *boundary*
heat transfer by exchanging boundary conditions. Cardinal provides three options
for these boundary conditions:
- Wall temperature
- Wall conductive heat flux (i.e. $-k\nabla T\cdot\hat{n}$)
- Wall convective heat flux (i.e. $h(T-T_\infty)$

This type of simulation is referred to as [!ac](CHT).
To set up a well-defined [!ac](CHT) case, you must choose one of these
boundary conditions for NekRS to send to MOOSE, and a *different* boundary condition
for MOOSE to send to NekRS. For example, you can build a [!ac](CHT) case which
passes temperature from NekRS to MOOSE and the wall conductive flux from MOOSE to NekRS.
This is shown below as the "Cond. Flux - Temperature" option.

!media cond_flux_temp.png
  id=cond_flux_temp
  caption=Illustration of [!ac](CHT) coupling with the "Cond. Flux - Temperature" option
  style=width:70%;margin-left:auto;margin-right:auto;halign:center

You could also build a [!ac](CHT) case which passes wall conductive heat flux from NekRS
to MOOSE and the wall temperature from MOOSE to NekRS. This is shown below as the
`Temperature - Cond. Flux` option.

!media temp_cond_flux.png
  id=temp_cond_flux
  caption=Illustration of [!ac](CHT) coupling with the "Temperature - Cond. Flux" option
  style=width:70%;margin-left:auto;margin-right:auto;halign:center

Another [!ac](CHT) option passes the wall convective heat flux from NekRS to MOOSE
and the wall conductive flux from MOOSE to NekRS. This is shown below as the
`Cond. Flux - Conv. Flux` option. Note that in all of these options, our naming convention
is to list the transfer *to* NekRS first, and the transfer *from* NekRS second. You might choose
to use different boundary conditions in order to improve the stability of the coupling.

!media cond_flux_conv_flux.png
  id=cond_flux_conv_flux
  caption=Illustration of [!ac](CHT) coupling with the "Cond. Flux - Conv. Flux" option
  style=width:70%;margin-left:auto;margin-right:auto;halign:center

Cardinal uses a general formulation that allows NekRS to couple via conjugate
heat transfer to *any* MOOSE thermal-fluid application. This
is shown schematically in [cardinal_cht]. All tutorials in this section couple
NekRS to the MOOSE heat transfer module, but the concepts extend equally to
coupling NekRS to any of these other MOOSE thermal-fluid codes.

!media cardinal_cht.png
  id=cardinal_cht
  caption=NekRS integrates with *any* MOOSE application that can compute a heat flux
  style=width:70%;margin-left:auto;margin-right:auto;halign:center

Five examples are provided; the "Pin Bundle Flow" example illustrates how to use the above
listed examples for [!ac](CHT) boundary conditions for easy comparison. For simplicity, all
other examples use the "Cond. Flux - Temperature" option.

- [Laminar Flow](cht5.md) over a heated pebble
- [Reflector Bypass Flow](cht1.md) around a reflector block in a pebble bed reactor
- [Pin Bundle Flow](cht2.md) in a bare 7-pin geometry
- [Turbulent RANS Channel Flow](cht3.md) in a heated [!ac](TRISO) compact
- [Turbulent LES Flow](cht4.md) in a heated 67-pebble [!ac](HTGR)
