# ComputeDiffusionCoeffMGAux

## Overview

`ComputeDiffusionCoeffMGAux` takes a Multi-Group (MG) total reaction rate variable (`total_rxn_rate`), a list of MG P1 scattering reaction rate variables (`p1_scatter_rxn_rates`), and a MG scalar flux variable (`scalar_flux`); these are then used to compute a particle diffusion coefficient with the
following equations:

\begin{equation}
\label{eq:mg_diffusion}
D_{i,g} = \frac{1}{3\Sigma_{tr,i,g}}
\end{equation}

\begin{equation}
\label{eq:mg_tr}
\Sigma_{tr,i,g} = \frac{\langle\sigma_{t}\psi\rangle_{i,g} - \sum_{g'}\langle\nu\sigma_{s}\psi\rangle_{i,1,g'\rightarrow g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where $\langle\sigma_{t}\psi\rangle_{i,g}$ is a MG total reaction rate, $\langle\nu\sigma_{s}\psi\rangle_{i,1,g'\rightarrow g}$ is a MG P1
scattering reaction rate, and $\langle\psi\rangle_{i,g}$ is a MG scalar flux. This AuxKernel is intended to be added via the MGXS block, see
[SetupMGXSAction](SetupMGXSAction.md) for more information regarding MG cross section generation.

## Example Input Syntax

The example below shows how `ComputeDiffusionCoeffMGAux` can be used to compute a MG diffusion coefficient:

!listing /test/tests/neutronics/gen_mgxs/mgxs_aux/mg_diffusion_coeff.i
  block=Problem

!syntax parameters /AuxKernels/ComputeDiffusionCoeffMGAux

!syntax inputs /AuxKernels/ComputeDiffusionCoeffMGAux
