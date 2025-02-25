# DimensionalizeAction

## Overview

The `DimensionalizeAction` action is responsible for fetching the characteristic scales from the user
and applying them to dimensionalize any field passed from NekRS into MOOSE (AuxVariables,
Postprocessors, UserObjects, etc.). To use, add a `[Dimensionalize]` block inside the `[Problem]` block.

Cardinal assumes that the NekRS inputs were nondimensionalized
with the following:

\begin{equation}
\label{eq:u_ref}
u_i^\dagger\equiv\frac{u_i}{u_{ref}}
\end{equation}

\begin{equation}
\label{eq:p_ref}
P^\dagger\equiv\frac{P}{\rho_0u_{ref}^2}
\end{equation}

\begin{equation}
\label{eq:T_ref}
T^\dagger\equiv\frac{T-T_{ref}}{\Delta T}
\end{equation}

\begin{equation}
\label{eq:x_ref}
x_i^\dagger\equiv\frac{x_i}{L_{ref}}
\end{equation}

\begin{equation}
\label{eq:t_ref}
t^\dagger\equiv\frac{t}{L_{ref}/u_{ref}}
\end{equation}

where $\dagger$ superscripts indicate nondimensional quantities.
`U` is used to specify $u_{ref}$, `T` is used to specify $T_{ref}$,
`dT` is used to specify $\Delta T$, `L` is used to specify $L_{ref}$,
`rho` is used to specify $\rho_0$, and `Cp` is used to specify $C_{p,0}$
(which does not appear above, but is necessary for scaling a volumetric heat source).
Finally, the mesh mirror must be in the same units as used in the coupled MOOSE application,
so the `scaling` parameter on [NekRSMesh](NekRSMesh.md) must be set to
dimensionalize the nondimensional `.re2` mesh. In other words,
`scaling` must be set to $L_{ref}$.

!alert warning
These characteristic scales are used by Cardinal to scale the NekRS solution
into the units that the coupled MOOSE application expects. *You* still need to properly
non-dimensionalize the NekRS input files. That is, you cannot
simply specify the non-dimensional scales in this class and expect a *dimsensional*
NekRS input specification to be converted to non-dimensional form.

For example, suppose your NekRS input is in non-dimensional form. Applying a
[NekVolumeAverage](NekVolumeAverage.md) postprocessor to temperature
would be used to evaluate a volume average of temperature.

!listing
[Postprocessors]
  [avg_T]
    type = NekVolumeAverage
    field = temperature
  []
[]

If the NekRS inputs are properly non-dimensionalized and the correct scales
are provided to this class, then temperature is non-dimensionalized according
to [eq:T_ref] and volume is non-dimensionalized according to [eq:x_ref], or

\begin{equation}
\label{eq:V_ref}
V^\dagger=\frac{V}{L_{ref}^3}
\end{equation}

The [NekVolumeAverage](NekVolumeAverage.md) postprocessor
is then computed directly on the NekRS solution (in non-dimensional form) to give

\begin{equation}
\label{eq:pp}
p_{nd}=\frac{\int T^\dagger dV^\dagger}{\int dV^\dagger}
\end{equation}

where $p_{nd}$ is the value of the postprocessor in non-dimensional form. Before returning the value
of the postprocessor, [eq:pp] is dimensionalized by applying the scales in [eq:T_ref]
and [eq:V_ref] to give the

\begin{equation}
\label{eq:pp_dim}
p=\frac{\int T^\dagger dV^\dagger}{\int dV^\dagger}+\frac{T_{ref}}{\Delta T}
\end{equation}

where $p$ is the value of the postprocessor in dimensional form (which
is what is actually returned by the postprocessor).

## Input Parameters

The following are all optional input parameters for this object:

- `U`: Reference velocity
- `L`: Reference length
- `T` and `dT`: Reference temperature and temperature delta, respectively
- `rho`: Reference density
- `Cp`: Reference isobaric specific heat

## Example Input File Syntax

An example where the NekRS input files are set up in non-dimensional form,

!listing /tutorials/msfr/nek.i
  block=Problem
