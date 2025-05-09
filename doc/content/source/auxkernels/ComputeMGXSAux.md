# ComputeMGXSAux

## Overview

`ComputeMGXSAux` takes a list of Multi-Group (MG) reaction rate variables (`rxn_rates`) and a list of MG normalization factor variables
(`normalize_by`); these are then used to compute an arbitrary MG cross section by dividing the sum of the reaction rate variables by the sum of the
variables in `normalize_by`. This AuxKernel is intended to be added via the MGXS block, see [SetupMGXSAction](SetupMGXSAction.md) for more information
regarding MG cross section generation.

## Example Input Syntax

The example below shows how `ComputeMGXSAux` can be used to compute a generic MG cross section:

!listing /test/tests/neutronics/gen_mgxs/mgxs_aux/generic_mgxs.i
  block=Problem

!syntax parameters /AuxKernels/ComputeMGXSAux

!syntax inputs /AuxKernels/ComputeMGXSAux
