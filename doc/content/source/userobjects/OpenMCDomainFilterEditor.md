# OpenMCDomainFilterEditor

!syntax description /UserObjects/OpenMCDomainFilterEditor

## Description

This user object can be used to create or control new domain filter in OpenMC.
Currently suppored filter types are "cell", "material", "universe", or "mesh"
with filter bins set by domain ID(s).

## Example Input Syntax

!listing test/tests/userobjects/openmc_tally_editor/add_filter.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCDomainFilterEditor

!syntax inputs /UserObjects/OpenMCDomainFilterEditor

!syntax children /UserObjects/OpenMCDomainFilterEditor