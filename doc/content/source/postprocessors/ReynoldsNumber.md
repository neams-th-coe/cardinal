# ReynoldsNumber

!syntax description /Postprocessors/ReynoldsNumber

## Description

This postprocessor computes the Reynolds number $Re$ as

\begin{equation}
\label{eq:re}
Re=\frac{\rho u_{ref}L_{ref}}{\mu}
\end{equation}

where $\rho$ is the fluid density, $u_{ref}$ is the characteristic velocity, $L_{ref}$ is
the characteristic length, and $\mu$ is the fluid dynamic viscosity. For non-dimensional
NekRS cases, this postprocessor can be helpful in confirming that you correctly set all
of the various non-dimensional scales in [NekRSProblem](/problems/NekRSProblem.md).

This postprocessor computes [eq:re] as

\begin{equation}
\label{re2}
Re=\frac{L_{ref}\int_\Gamma \rho \vec{V}\cdot\hat{n}d\Gamma}{\mu\int_\Gamma d\Gamma}
\end{equation}

where $\Gamma$ is a boundary of the NekRS mesh specified with the `boundary` parameter
and $\vec{V}$ is the fluid velocity. If NekRS is run in non-dimensional form, the
characteristic length $L_{ref}$ is equal to the `L_ref` parameter set in
[NekRSProblem](/problems/NekRSProblem.md). If NekRS is instead run in dimensional form,
the characteristic length must be provided in this postprocessor with the `L_ref` parameter.

!alert warning
This postprocessor currently assumes that $\rho$ and $\mu$ are both constant.

## Example Input Syntax

As an example for a dimensional NekRS case, the `Re` postprocessor will compute the Reynolds number according
to the velocity through boundary 1 of the NekRS mesh.

!listing test/tests/postprocessors/reynolds_number/dimensional/nek.i
  block=Postprocessors

As an example for a non-dimensional NekRS case, the `Re` postprocessor will compute the Reynolds number according
to the velocity through boundary 1 of the NekRS mesh.

!listing test/tests/postprocessors/reynolds_number/nondimensional/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/ReynoldsNumber

!syntax inputs /Postprocessors/ReynoldsNumber

!syntax children /Postprocessors/ReynoldsNumber
