# Solid Temperature Coupling of OpenMC and MOOSE

This tutorial describes how to couple MOOSE and OpenMC for heat source
and temperature coupling. Two examples are provided - a [!ac](LWR) pincell
and a [!ac](TRISO) pebble. Many of the features that are introduced in these
tutorials are *general* features that also apply to fluid feedback in OpenMC.
For instance, in the [TRISO pebble](triso.md) tutorial, we describe how to
couple an OpenMC solution in centimeters with a MOOSE solution in meters. This
feature is equally applicable to both solid and fluid feedback in OpenMC,
even though the examples here only focus on solid feedback. This allows
the tutorials to slowly progress in complexity. Any capabilities that
*truly* are specific to solid temperature feedback will be indicated where applicable.

- [LWR pincell](pincell1.md)
- [TRISO pebble](triso.md)
