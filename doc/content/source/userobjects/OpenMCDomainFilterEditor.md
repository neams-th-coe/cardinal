# OpenMCDomainFilterEditor

!syntax description /UserObjects/OpenMCDomainFilterEditor

## Description

This user object can be used to create or control new domain filter in OpenMC.
Currently suppored filter types are specified under the "filter_type" parameter.
The filter bins are set using domain ID(s) in the OpenMC model.

## Example Input Syntax

!listing test/tests/userobjects/openmc_tally_editor/add_filter.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCDomainFilterEditor

!syntax inputs /UserObjects/OpenMCDomainFilterEditor

!syntax children /UserObjects/OpenMCDomainFilterEditor