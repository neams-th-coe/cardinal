# PecletNumber

!syntax description /Postprocessors/PecletNumber

## Description

This postprocessor computes the Peclet number $Pe$ as

\begin{equation}
\label{eq:re}
Pe=\frac{\rho C_pu_{ref}L_{ref}}{k}
\end{equation}

where $\rho$ is the fluid density, $C_p$ is the fluid isobaric specific
heat capacity, $u_{ref}$ is the characteristic velocity, $L_{ref}$ is
the characteristic length, and $k$ is the fluid thermal conductivity. For non-dimensional
NekRS cases, this postprocessor can be helpful in confirming that you correctly set all
of the various non-dimensional scales in [NekRSProblem](NekRSProblem.md).

This postprocessor computes [eq:re] as

\begin{equation}
\label{re2}
Pe=\frac{L_{ref}\int_\Gamma \rho C_p \vec{V}\cdot\hat{n}d\Gamma}{k\int_\Gamma d\Gamma}
\end{equation}

where $\Gamma$ is a boundary of the NekRS mesh specified with the `boundary` parameter
and $\vec{V}$ is the fluid velocity. If NekRS is run in non-dimensional form, the
characteristic length $L_{ref}$ is equal to the `L_ref` parameter set in
[NekRSProblem](NekRSProblem.md). If NekRS is instead run in dimensional form,
the characteristic length must be provided in this postprocessor with the `L_ref` parameter.

!alert warning
This postprocessor currently assumes that $\rho$, $C_p$, and $k$ are all constant.

## Example Input Syntax

As an example for a dimensional NekRS case, the `Pe` postprocessor will compute the Peclet number according
to the velocity through boundary 1 of the NekRS mesh.

!listing test/tests/postprocessors/dimensionless_numbers/dimensional/nek.i
  block=Postprocessors

As an example for a non-dimensional NekRS case, the `Pe` postprocessor will compute the Peclet number according
to the velocity through boundary 1 of the NekRS mesh.

!listing test/tests/postprocessors/dimensionless_numbers/nondimensional/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/PecletNumber

!syntax inputs /Postprocessors/PecletNumber
