# HeatTransferCoefficientAux

!syntax description /AuxKernels/HeatTransferCoefficientAux

## Description

Helper auxiliary kernel to compute a heat transfer coefficient as

\begin{equation}
q^{''}=h(T-T_\infty)
\end{equation}

where $q^{''}$, $T$, and $T_\infty$ are each provided by a binned user
object which computes these quantities as a function of position.
$q^{''}$ and $T$ should each be computed using a [NekBinnedSideAverage](NekBinnedSideAverage.md)
user object, since these terms represent area integrals over the heated surface,
while $T_\infty$ should be computed using a [NekBinnedVolumeAverage](NekBinnedVolumeAverage.md) user object, since this term represents a volume integral over
the heated fluid volume.

## Example Input Syntax

The example below shows how to correctly compute a heat transfer coefficient.

!listing test/tests/auxkernels/heat_transfer_coefficient/nek.i
  block=UserObjects

!syntax parameters /AuxKernels/HeatTransferCoefficientAux

!syntax inputs /AuxKernels/HeatTransferCoefficientAux
