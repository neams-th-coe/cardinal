# FoMAux

!syntax description /AuxKernels/FoMAux

## Description

This auxkernel computes a Monte Carlo [!ac](FoM). There are two options, the first is the
[!ac](VR) [!ac](FoM):

\begin{equation}
\label{eq:vr_fom}
\text{FoM}_{VR,i} = \frac{1}{T R_{i}^{2}}
\end{equation}

where $T$ is the total simulation time (time spent running the Monte Carlo calculation and
performing variance reduction), and $R_{i}$ is the stochastic relative error in tally bin $i$.
The second option is the [!ac](AMR) [!ac](FoM):

\begin{equation}
\label{eq:amr_fom}
\text{FoM}_{AMR,i} = \frac{1}{T_{j} R_{j,i}}\frac{|u_{j,i} - u_{0,i}|}{u_{0,i}}
\end{equation}

where $T_{j}$ is the cumulative simulation time spent up to [!ac](AMR) cycle $j$, $u_{j,i}$ is the
tally value at [!ac](AMR) cycle $j$ in tally bin $i$, $u_{0,i}$ is the tally value  on the initial
(non-adapted) calculation, and $R_{j,i}$ is the tally stochastic relative error at [!ac](AMR) cycle
$j$ in tally bin $i$.

The [!ac](FoM) computed can be selected by setting [!param](/AuxKernels/FoMAux/fom_type).
When computing the [!ac](VR) [!ac](FoM), [!param](/AuxKernels/FoMAux/sim_time) and
[!param](/AuxKernels/FoMAux/tally_rel_error) must be provided. When computing the [!ac](AMR) [!ac](FoM),
[!param](/AuxKernels/FoMAux/sim_time), [!param](/AuxKernels/FoMAux/tally_rel_error),
[!param](/AuxKernels/FoMAux/tally_value_init), and [!param](/AuxKernels/FoMAux/tally_value) must be
provided.

## Example Input Syntax

As an example, the first syntax block below computes the [!ac](VR) [!ac](FoM) while the second block computes the
[!ac](AMR) [!ac](FoM).

!listing test/tests/neutronics/foms/vr.i
  block=AuxKernels

!listing test/tests/neutronics/foms/amr.i
  block=AuxKernels

!syntax parameters /AuxKernels/FoMAux

!syntax inputs /AuxKernels/FoMAux
