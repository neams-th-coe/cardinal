# ComputeTCScatterMGXSAux

## Overview

`ComputeTCScatterMGXSAux` takes a Multi-Group (MG) isotropic scattering reaction rate variable (`p0_scatter_rxn_rate`), a list of
MG P1 scattering reaction rate variables (`p1_scatter_rxn_rates`), and a MG scalar flux variable (`scalar_flux`); these are then
used to compute a MG transport corrected isotropic scattering cross section with the following equations:

## Example Input Syntax

The example below shows how `ComputeTCScatterMGXSAux` can be used to compute a MG transport corrected scattering cross section:

\begin{equation}
\label{eq:mg_scatter_tc_xs}
\Sigma_{s,i,0,g'\rightarrow g} = \frac{\langle\sigma_{s}\psi\rangle_{i,0,g\rightarrow g'} - \delta_{gg'}\sum_{g''}\langle\sigma_{s}\psi\rangle_{i,1,g''\rightarrow g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where $\langle\sigma_{s}\psi\rangle_{i,0,g\rightarrow g'}$ is a within-group isotropic scattering cross section, $\langle\nu\sigma_{s}\psi\rangle_{i,1,g'\rightarrow g}$ is a MG P1 scattering reaction rate, and $\langle\psi\rangle_{i,g}$ is a MG scalar flux. This AuxKernel is intended to be added via the MGXS block,
see [SetupMGXSAction](SetupMGXSAction.md) for more information regarding MG cross section generation.

The example below shows how `ComputeTCScatterMGXSAux` can be used to compute a MG transport corrected isotropic scattering cross section:

!listing /test/tests/neutronics/gen_mgxs/mgxs_aux/mg_tc_scatter_xs.i
  block=Problem

!syntax parameters /AuxKernels/ComputeTCScatterMGXSAux

!syntax inputs /AuxKernels/ComputeTCScatterMGXSAux
