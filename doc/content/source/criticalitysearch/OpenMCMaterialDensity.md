# OpenMCMaterialDensity

## Description

`OpenMCMaterialDensity` is a [CriticalitySearch](AddCriticalitySearchAction.md) that
performs a criticality search based on the total density of a provided material.

## Example Input File Syntax

The following input will perform a criticality search for each OpenMC solve
by searching via the density of material ID 1.

!listing test/tests/criticality/material_density
  block=Problem

!syntax parameters /Problem/CriticalitySearch/OpenMCMaterialDensity

!syntax inputs /Problem/CriticalitySearch/OpenMCMaterialDensity
