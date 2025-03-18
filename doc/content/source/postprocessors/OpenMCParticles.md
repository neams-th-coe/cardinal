# OpenMCParticles

!syntax description /Postprocessors/OpenMCParticles

## Description

This postprocessor extracts the number of particles which have been run in OpenMC,
as the number of particles/batch times the number of batches.

## Example Input Syntax

Shown below is an example for depicting the instantaneous and total number of particles simulated.

!listing test/tests/postprocessors/openmc_particles/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/OpenMCParticles

!syntax inputs /Postprocessors/OpenMCParticles
