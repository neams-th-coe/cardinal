# Solid Coupling of OpenMC and MOOSE

This tutorial describes how to couple MOOSE and OpenMC for heat source
and temperature feedback. Three examples are provided - a [!ac](LWR) pincell,
solid UO$_2$ pebbles in a lattice, and a [!ac](TRISO) fuel gas reactor compact.
Many of the features that are introduced in these
tutorials are *general* features that also apply to fluid feedback in OpenMC.
For instance, in the [pebble tutorial](triso.md), we describe how to
couple an OpenMC solve in centimeters with a MOOSE solve in meters. This
feature is equally applicable to both solid and fluid feedback in OpenMC,
even though the examples here only focus on solid feedback. This allows
the tutorials to slowly progress in complexity. Any capabilities that
*truly* are specific to solid temperature feedback will be indicated where applicable.

A. [LWR pincell](pincell1.md)

B. [Solid UO$_2$ pebbles](triso.md)

C. [TRISO compacts](gas_compact.md)

!alert note
These tutorials assume some familiarity with OpenMC. As such, the emphasis is on
multiphysics coupling with Cardinal. Please refer to the
[OpenMC documentation](https://docs.openmc.org/en/stable/) for information on
setting up the OpenMC models.
