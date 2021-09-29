# Tutorial 9A: Multiphysics Coupling for an SFR Pin Bundle

In this tutorial, you will learn how to:

- Couple OpenMC, NekRS, and MOOSE for steady-state multiphysics simulations

This tutorial describes how to use Cardinal to perform temperature, density,
and fission power coupling of OpenMC, NekRS, and MOOSE for a 7-pin [!ac](SFR)
pin bundle. This tutorial is an adaptation of the [!ac](SFR) modeling
performed in [!cite](novak2022). For pedagogical purposes,
several simplifications are made
in the formulation of the geometry, material properties, and operating conditions
so that the emphasis of the computational model remains focused on the
multiphysics coupling and data transfers.

# Geometry and Computational Model

This section describes the geometry for a 7-pin [!ac](SFR) bundle. This geometry
is a reduced-size version of the 271-pin [!ac](ABR) metal driver fuel assemblies [!cite](abr),
with a few additional simplifications.
[table1] summarizes the relevant dimensions of the geometry with a comparison to the nominal
[!ac](ABR) design.

!table id=table1 caption=Geometric conditions for the [!ac](ABR), based on [!cite](abr)
| Parameter | Value Used in Tutorial (cm) | Nominal ABR Value (cm) |
| :- | :- | :- |
| Pellet diameter | 0.7646 | 0.7647 |
| Pin pitch | 0.8966 | 0.8966 |
| Clad thickness $t_c$ | 0.0587 | 0.0587 |
| Wire diameter, $D_w$ | 0.103 | 0.103 |
| Wire axial pitch, $L_w$ | 20.32 | 20.32 |
| Duct thickness | 0.1983 | 0.3966 |
| Rod plenum height | 0.0 | 121.07 |
| Top reflector height | 25.0 | 112.39 |
| Bottom reflector height | 25.0 | 35.76 |
| Active height | 60.96 | 85.82 |

# MultiApps and Transfers
