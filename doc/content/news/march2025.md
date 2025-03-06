# March 2025 News

- We have added a tutorial showing how to couple NekRS and MOOSE via [!ac](CHT), using a different set of boundary conditions. Up until now, the [!ac](CHT) coupling passed MOOSE wall conductive heat flux to NekRS as a Neumann condition, while NekRS passes wall temperature to MOOSE as a Dirichlet condition. Now, we also support flux-flux coupling, where NekRS passes $h(T-T_\infty)$ to MOOSE as a Robin convective boundary condition. This option may be more stable and allow more extensive subcycling (more NekRS time steps per MOOSE time step).

!media cht.png
  id=sfr_cht
  caption=Illustration of data transfers to couple NekRS to MOOSE for boundary heat transfer in a pin bundle.
  style=width:70%;margin-left:auto;margin-right:auto;halign:center
